import AppKit
import Combine
import Foundation
import SweetLine

@MainActor
final class DemoViewModel: ObservableObject {
    @Published var status = "Preparing syntax rules..."
    @Published var isWarmingUp = false
    @Published var exampleFiles: [String] = []
    @Published var selectedFileName = ""
    @Published var selectedThemeIndex = 0
    @Published var sourceCode = ""
    @Published var highlight: DocumentHighlight?
    @Published var indentGuides: IndentGuideResult?
    @Published var bracketPairs: BracketPairResult?

    let themes = HighlightTheme.builtinThemes()

    private static let yamlNonZeroWidthFile = "yaml(non zero width).json"
    private static let syntaxSampleFile = "json-sweetline.json"

    private var engine: HighlightEngine?
    private var syntaxDirectory: URL?
    private var examplesDirectory: URL?
    private var compiledSyntaxCount = 0
    private var warmupElapsedMillis = 0
    private var didWarmup = false
    private var suppressSelectionChange = false

    var currentTheme: HighlightTheme {
        themes.indices.contains(selectedThemeIndex) ? themes[selectedThemeIndex] : themes[0]
    }

    deinit {
        engine?.close()
    }

    func warmupIfNeeded() async {
        guard !didWarmup else { return }
        didWarmup = true
        isWarmingUp = true
        defer { isWarmingUp = false }

        do {
            let repoRoot = try Self.findRepoRoot()
            syntaxDirectory = repoRoot.appendingPathComponent("syntaxes", isDirectory: true)
            examplesDirectory = repoRoot.appendingPathComponent("tests/files", isDirectory: true)

            guard let syntaxDirectory, Self.isDirectory(syntaxDirectory) else {
                throw DemoError.missingDirectory("syntaxes")
            }
            guard let examplesDirectory, Self.isDirectory(examplesDirectory) else {
                throw DemoError.missingDirectory("tests/files")
            }

            let startedAt = DispatchTime.now().uptimeNanoseconds
            let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
            self.engine = engine
            try registerStyleNames(engine)
            try engine.defineMacro("WINDOWS")

            let syntaxSources = try listCommonSyntaxSources(in: syntaxDirectory)
            compiledSyntaxCount = try compileSyntaxSources(syntaxSources, engine: engine)
            exampleFiles = try listExampleFiles(examplesDirectory: examplesDirectory, syntaxesDirectory: syntaxDirectory, engine: engine)
            warmupElapsedMillis = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000_000)

            if let first = exampleFiles.first {
                suppressSelectionChange = true
                selectedFileName = first
                suppressSelectionChange = false
                highlightSelectedFile(first)
            } else {
                status = "Compiled \(compiledSyntaxCount) syntax rule files in \(warmupElapsedMillis) ms | No demo files available"
            }
        } catch {
            status = "Warmup failed: \(error)"
        }
    }

    func highlightSelectedFile(_ fileName: String) {
        guard !suppressSelectionChange, !fileName.isEmpty else { return }
        guard let url = resolveDemoSampleURL(fileName) else {
            status = "Example file not found: \(fileName)"
            return
        }
        highlightFile(url, documentFileName: fileName)
    }

    func openExternalFile() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false

        guard panel.runModal() == .OK, let url = panel.url else {
            return
        }

        suppressSelectionChange = true
        selectedFileName = ""
        suppressSelectionChange = false
        highlightFile(url, documentFileName: url.lastPathComponent)
    }

    private func highlightFile(_ fileURL: URL, documentFileName: String) {
        guard let engine else {
            status = "Engine is not ready."
            return
        }

        do {
            let source = try String(contentsOf: fileURL, encoding: .utf8)
            let document = try Document(uri: documentFileName, text: source)
            defer { document.close() }

            guard let analyzer = try engine.loadDocument(document) else {
                status = "No matching syntax for file: \(documentFileName)"
                return
            }
            defer { analyzer.close() }

            let startedAt = DispatchTime.now().uptimeNanoseconds
            let documentHighlight = try analyzer.analyze()
            let analyzeMicros = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000)
            let guides = try analyzer.analyzeIndentGuides()
            let pairs = try analyzer.analyzeBracketPairs()

            sourceCode = source
            highlight = documentHighlight
            indentGuides = guides
            bracketPairs = pairs

            let bracketTokenCount = pairs.lines.reduce(0) { $0 + $1.tokens.count }
            status = "Warmup: \(compiledSyntaxCount) files in \(warmupElapsedMillis) ms | Analyze: \(formatMicros(analyzeMicros)) | Lines: \(lineCount(source)) | Brackets: \(bracketTokenCount) | File: \(fileURL.lastPathComponent)"
        } catch {
            status = "Error: \(error)"
            bracketPairs = nil
        }
    }

    private func listCommonSyntaxSources(in directory: URL) throws -> [SyntaxSource] {
        let urls = try FileManager.default.contentsOfDirectory(
            at: directory,
            includingPropertiesForKeys: [.isRegularFileKey],
            options: [.skipsHiddenFiles]
        )

        return try urls
            .filter { $0.pathExtension == "json" && shouldPrecompileSyntaxFile($0.lastPathComponent) }
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
            .map { url in
                SyntaxSource(fileName: url.lastPathComponent, json: try String(contentsOf: url, encoding: .utf8))
            }
    }

    private func compileSyntaxSources(_ sources: [SyntaxSource], engine: HighlightEngine) throws -> Int {
        var pending = sources
        var compiledCount = 0

        while !pending.isEmpty {
            var progressed = false
            var nextPending: [SyntaxSource] = []

            for source in pending {
                do {
                    try engine.compileSyntax(fromJson: source.json)
                    compiledCount += 1
                    progressed = true
                    status = "Compiling \(compiledCount)/\(sources.count): \(source.fileName)"
                } catch let error as SyntaxCompileError where error.code == SyntaxCompileError.importSyntaxNotFound {
                    nextPending.append(source)
                } catch let error as SyntaxCompileError {
                    throw DemoError.syntaxCompile("Failed to compile \(source.fileName): \(error.message)")
                }
            }

            if !progressed {
                let unresolved = nextPending.map(\.fileName).joined(separator: ", ")
                throw DemoError.syntaxCompile("Unresolved importSyntax dependencies: \(unresolved)")
            }

            pending = nextPending
        }

        return compiledCount
    }

    private func listExampleFiles(examplesDirectory: URL, syntaxesDirectory: URL, engine: HighlightEngine) throws -> [String] {
        var files = try FileManager.default.contentsOfDirectory(
            at: examplesDirectory,
            includingPropertiesForKeys: [.isRegularFileKey],
            options: [.skipsHiddenFiles]
        )
        .filter { supportsFileNameRouting($0.lastPathComponent, engine: engine) }
        .map(\.lastPathComponent)

        let syntaxSample = syntaxesDirectory.appendingPathComponent(Self.syntaxSampleFile)
        if FileManager.default.isReadableFile(atPath: syntaxSample.path), supportsFileNameRouting(Self.syntaxSampleFile, engine: engine) {
            files.append(Self.syntaxSampleFile)
        }

        return files.sorted()
    }

    private func resolveDemoSampleURL(_ fileName: String) -> URL? {
        if let examplesDirectory {
            let example = examplesDirectory.appendingPathComponent(fileName)
            if FileManager.default.isReadableFile(atPath: example.path) {
                return example
            }
        }
        if let syntaxDirectory {
            let syntax = syntaxDirectory.appendingPathComponent(fileName)
            if FileManager.default.isReadableFile(atPath: syntax.path) {
                return syntax
            }
        }
        return nil
    }

    private func supportsFileNameRouting(_ fileName: String, engine: HighlightEngine) -> Bool {
        do {
            let analyzer = try engine.createAnalyzer(fileName: fileName)
            analyzer?.close()
            return analyzer != nil
        } catch {
            return false
        }
    }

    private func shouldPrecompileSyntaxFile(_ fileName: String) -> Bool {
        !fileName.hasSuffix("-inlineStyle.json") && fileName != Self.yamlNonZeroWidthFile
    }

    private func registerStyleNames(_ engine: HighlightEngine) throws {
        try engine.registerStyleName("keyword", id: HighlightTheme.styleKeyword)
        try engine.registerStyleName("string", id: HighlightTheme.styleString)
        try engine.registerStyleName("number", id: HighlightTheme.styleNumber)
        try engine.registerStyleName("comment", id: HighlightTheme.styleComment)
        try engine.registerStyleName("class", id: HighlightTheme.styleClass)
        try engine.registerStyleName("method", id: HighlightTheme.styleMethod)
        try engine.registerStyleName("variable", id: HighlightTheme.styleVariable)
        try engine.registerStyleName("punctuation", id: HighlightTheme.stylePunctuation)
        try engine.registerStyleName("annotation", id: HighlightTheme.styleAnnotation)
        try engine.registerStyleName("preprocessor", id: HighlightTheme.stylePreprocessor)
        try engine.registerStyleName("macro", id: HighlightTheme.styleMacro)
        try engine.registerStyleName("lifetime", id: HighlightTheme.styleLifetime)
        try engine.registerStyleName("selector", id: HighlightTheme.styleSelector)
        try engine.registerStyleName("builtin", id: HighlightTheme.styleBuiltin)
        try engine.registerStyleName("url", id: HighlightTheme.styleURL)
        try engine.registerStyleName("property", id: HighlightTheme.styleProperty)
    }

    private func lineCount(_ source: String) -> Int {
        source.components(separatedBy: "\n").count
    }

    private func formatMicros(_ micros: Int) -> String {
        guard micros >= 1_000 else { return "\(micros)us" }

        let millis = Double(micros) / 1_000.0
        if millis < 10 {
            return String(format: "%.2fms", millis)
        }
        if millis < 100 {
            return String(format: "%.1fms", millis)
        }
        return String(format: "%.0fms", millis)
    }

    private static func findRepoRoot() throws -> URL {
        var url = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
        while url.path != "/" {
            if isDirectory(url.appendingPathComponent("syntaxes", isDirectory: true)) &&
                isDirectory(url.appendingPathComponent("tests/files", isDirectory: true)) {
                return url
            }
            url.deleteLastPathComponent()
        }
        throw DemoError.missingDirectory("repository root containing syntaxes and tests/files")
    }

    private static func isDirectory(_ url: URL) -> Bool {
        var isDirectory: ObjCBool = false
        return FileManager.default.fileExists(atPath: url.path, isDirectory: &isDirectory) && isDirectory.boolValue
    }
}

private struct SyntaxSource {
    let fileName: String
    let json: String
}

private enum DemoError: Error, CustomStringConvertible {
    case missingDirectory(String)
    case syntaxCompile(String)

    var description: String {
        switch self {
        case let .missingDirectory(path):
            return "Missing directory: \(path)"
        case let .syntaxCompile(message):
            return message
        }
    }
}
