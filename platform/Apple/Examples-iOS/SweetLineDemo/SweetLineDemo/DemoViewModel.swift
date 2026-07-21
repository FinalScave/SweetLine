import Combine
import Foundation
import SweetLine

struct DemoSample: Identifiable, Hashable {
    let displayName: String
    let fileName: String
    let url: URL
    let inlineStyle: Bool

    var id: String { inlineStyle ? "inline:\(fileName)" : "common:\(fileName)" }
}

@MainActor
final class DemoViewModel: ObservableObject {
    @Published var status = "Preparing syntax rules..."
    @Published var isWarmingUp = false
    @Published var demoSamples: [DemoSample] = []
    @Published var selectedSampleID = ""
    @Published var selectedThemeIndex = 0
    @Published var sourceCode = ""
    @Published var highlight: DocumentHighlight?
    @Published var indentGuides: IndentGuideResult?
    @Published var bracketPairs: BracketPairResult?

    let themes = HighlightTheme.builtinThemes()

    private static let yamlNonZeroWidthFile = "yaml(non zero width).json"
    private static let syntaxSampleFile = "json-sweetline.json"
    private static let inlineStyleSampleFiles: Set<String> = ["example.java", "example.t"]
    private static let inlineStyleSyntaxFiles = ["java-inlineStyle.json", "tiecode-inlineStyle.json"]

    private var commonEngine: HighlightEngine?
    private var inlineStyleEngine: HighlightEngine?
    private var currentDocument: Document?
    private var currentAnalyzer: DocumentAnalyzer?
    private var currentDocumentName = ""
    private var currentSampleIsInlineStyle = false
    private var syntaxDirectory: URL?
    private var examplesDirectory: URL?
    private var compiledSyntaxCount = 0
    private var warmupElapsedMillis = 0
    private var didWarmup = false
    private var suppressSelectionChange = false

    var currentTheme: HighlightTheme {
        themes.indices.contains(selectedThemeIndex) ? themes[selectedThemeIndex] : themes[0]
    }

    func warmupIfNeeded() async {
        guard !didWarmup else { return }
        didWarmup = true
        isWarmingUp = true
        defer { isWarmingUp = false }

        do {
            syntaxDirectory = try Self.requiredBundleDirectory("syntaxes")
            examplesDirectory = try Self.requiredBundleDirectory("files")

            guard let syntaxDirectory, let examplesDirectory else {
                throw DemoError.missingResource("bundled syntaxes or files directory")
            }

            let startedAt = DispatchTime.now().uptimeNanoseconds

            let commonEngine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
            self.commonEngine = commonEngine
            try registerStyleNames(commonEngine)
            try commonEngine.defineMacro("ANDROID")

            let inlineStyleEngine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: true))
            self.inlineStyleEngine = inlineStyleEngine
            try inlineStyleEngine.defineMacro("ANDROID")

            let commonSyntaxSources = try listCommonSyntaxSources(in: syntaxDirectory)
            let inlineStyleSyntaxSources = try listInlineStyleSyntaxSources(in: syntaxDirectory)
            let totalSyntaxCount = commonSyntaxSources.count + inlineStyleSyntaxSources.count

            var compiledCount = try compileSyntaxSources(
                commonSyntaxSources,
                engine: commonEngine,
                compiledOffset: 0,
                totalCount: totalSyntaxCount
            )
            compiledCount += try compileSyntaxSources(
                inlineStyleSyntaxSources,
                engine: inlineStyleEngine,
                compiledOffset: compiledCount,
                totalCount: totalSyntaxCount
            )
            compiledSyntaxCount = compiledCount

            demoSamples = try listDemoSamples(examplesDirectory: examplesDirectory, syntaxesDirectory: syntaxDirectory)
            warmupElapsedMillis = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000_000)

            if let first = demoSamples.first {
                suppressSelectionChange = true
                selectedSampleID = first.id
                suppressSelectionChange = false
                highlightSelectedSample(first.id)
            } else {
                status = "Compiled \(compiledSyntaxCount) syntax rule files in \(warmupElapsedMillis) ms | No demo files available"
            }
        } catch {
            status = "Warmup failed: \(error)"
        }
    }

    func highlightSelectedSample(_ sampleID: String) {
        guard !suppressSelectionChange, !sampleID.isEmpty else { return }
        guard let sample = demoSamples.first(where: { $0.id == sampleID }) else {
            status = "Example file not found: \(sampleID)"
            return
        }
        highlightSample(sample)
    }

    func openImportedFile(_ url: URL) {
        let shouldStopAccessing = url.startAccessingSecurityScopedResource()
        defer {
            if shouldStopAccessing {
                url.stopAccessingSecurityScopedResource()
            }
        }

        suppressSelectionChange = true
        selectedSampleID = ""
        suppressSelectionChange = false
        highlightFile(url, documentFileName: url.lastPathComponent, inlineStyle: false)
    }

    func applyTextEdit(range: NSRange, replacementText: String) {
        guard let swiftRange = Range(range, in: sourceCode) else {
            status = "Edit failed: invalid text range"
            return
        }

        let oldSource = sourceCode
        let newSource = oldSource.replacingCharacters(in: swiftRange, with: replacementText)
        let changeRange = textRange(in: oldSource, replacing: swiftRange)

        guard let analyzer = currentAnalyzer else {
            sourceCode = newSource
            highlight = nil
            indentGuides = nil
            bracketPairs = nil
            return
        }

        do {
            let startedAt = DispatchTime.now().uptimeNanoseconds
            let updatedHighlight = try analyzer.analyzeIncremental(range: changeRange, newText: replacementText)
            let analyzeMicros = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000)
            let guides = try analyzer.analyzeIndentGuides()
            let pairs = try analyzer.analyzeBracketPairs()

            sourceCode = newSource
            highlight = updatedHighlight
            indentGuides = guides
            bracketPairs = pairs
            let bracketTokenCount = pairs.lines.reduce(0) { $0 + $1.tokens.count }
            status = "Warmup: \(compiledSyntaxCount) files in \(warmupElapsedMillis) ms | Incremental: \(formatMicros(analyzeMicros)) | Lines: \(lineCount(newSource)) | Brackets: \(bracketTokenCount) | File: \(currentDocumentName)"
        } catch {
            sourceCode = newSource
            status = "Incremental failed, reloading: \(error)"
            reloadCurrentDocumentFromSource(newSource)
        }
    }

    private func highlightSample(_ sample: DemoSample) {
        highlightFile(sample.url, documentFileName: sample.fileName, inlineStyle: sample.inlineStyle)
    }

    private func highlightFile(_ fileURL: URL, documentFileName: String, inlineStyle: Bool) {
        let engine = inlineStyle ? inlineStyleEngine : commonEngine
        guard let engine else {
            status = "Engine is not ready."
            return
        }

        do {
            let source = try String(contentsOf: fileURL, encoding: .utf8)
            loadSource(source, documentFileName: documentFileName, engine: engine, inlineStyle: inlineStyle, displayFileName: fileURL.lastPathComponent)
        } catch {
            status = "Error: \(error)"
        }
    }

    private func loadSource(
        _ source: String,
        documentFileName: String,
        engine: HighlightEngine,
        inlineStyle: Bool,
        displayFileName: String
    ) {
        do {
            closeCurrentDocument()

            let document = try Document(uri: documentFileName, text: source)
            guard let analyzer = try engine.loadDocument(document) else {
                document.close()
                currentDocumentName = documentFileName
                currentSampleIsInlineStyle = inlineStyle
                sourceCode = source
                highlight = nil
                indentGuides = nil
                bracketPairs = nil
                status = "No matching syntax for file: \(documentFileName)"
                return
            }

            let startedAt = DispatchTime.now().uptimeNanoseconds
            let documentHighlight = try analyzer.analyze()
            let analyzeMicros = Int((DispatchTime.now().uptimeNanoseconds - startedAt) / 1_000)
            let guides = try analyzer.analyzeIndentGuides()
            let pairs = try analyzer.analyzeBracketPairs()

            currentDocument = document
            currentAnalyzer = analyzer
            currentDocumentName = documentFileName
            currentSampleIsInlineStyle = inlineStyle
            sourceCode = source
            highlight = documentHighlight
            indentGuides = guides
            bracketPairs = pairs

            let mode = inlineStyle ? " [inline]" : ""
            let bracketTokenCount = pairs.lines.reduce(0) { $0 + $1.tokens.count }
            status = "Warmup: \(compiledSyntaxCount) files in \(warmupElapsedMillis) ms | Analyze: \(formatMicros(analyzeMicros)) | Lines: \(lineCount(source)) | Brackets: \(bracketTokenCount) | File: \(displayFileName)\(mode)"
        } catch {
            closeCurrentDocument()
            sourceCode = ""
            highlight = nil
            indentGuides = nil
            bracketPairs = nil
            status = "Error: \(error)"
        }
    }

    private func reloadCurrentDocumentFromSource(_ source: String) {
        let inlineStyle = currentSampleIsInlineStyle
        let documentName = currentDocumentName
        let engine = inlineStyle ? inlineStyleEngine : commonEngine
        guard let engine else { return }
        loadSource(source, documentFileName: documentName, engine: engine, inlineStyle: inlineStyle, displayFileName: documentName)
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

    private func listInlineStyleSyntaxSources(in directory: URL) throws -> [SyntaxSource] {
        try Self.inlineStyleSyntaxFiles.map { fileName in
            let url = directory.appendingPathComponent(fileName)
            return SyntaxSource(fileName: fileName, json: try String(contentsOf: url, encoding: .utf8))
        }
    }

    private func compileSyntaxSources(
        _ sources: [SyntaxSource],
        engine: HighlightEngine,
        compiledOffset: Int,
        totalCount: Int
    ) throws -> Int {
        var pending = sources
        var compiledCount = 0

        while !pending.isEmpty {
            var progressed = false
            var nextPending: [SyntaxSource] = []

            for source in pending {
                status = "Compiling \(compiledOffset + compiledCount + 1)/\(totalCount): \(source.fileName)"
                do {
                    try engine.compileSyntax(fromJson: source.json)
                    compiledCount += 1
                    progressed = true
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

    private func listDemoSamples(examplesDirectory: URL, syntaxesDirectory: URL) throws -> [DemoSample] {
        var samples: [DemoSample] = []
        let exampleURLs = try FileManager.default.contentsOfDirectory(
            at: examplesDirectory,
            includingPropertiesForKeys: [.isRegularFileKey],
            options: [.skipsHiddenFiles]
        )
        .sorted { $0.lastPathComponent < $1.lastPathComponent }

        for url in exampleURLs {
            let fileName = url.lastPathComponent
            samples.append(DemoSample(displayName: fileName, fileName: fileName, url: url, inlineStyle: false))
            if Self.inlineStyleSampleFiles.contains(fileName) {
                samples.append(DemoSample(displayName: "\(fileName) [inline]", fileName: fileName, url: url, inlineStyle: true))
            }
        }

        let syntaxSample = syntaxesDirectory.appendingPathComponent(Self.syntaxSampleFile)
        if FileManager.default.isReadableFile(atPath: syntaxSample.path) {
            samples.append(DemoSample(
                displayName: Self.syntaxSampleFile,
                fileName: Self.syntaxSampleFile,
                url: syntaxSample,
                inlineStyle: false
            ))
        }

        return samples
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

    private func closeCurrentDocument() {
        currentAnalyzer?.close()
        currentAnalyzer = nil
        currentDocument?.close()
        currentDocument = nil
    }

    private func textRange(in text: String, replacing range: Range<String.Index>) -> TextRange {
        TextRange(
            start: textPosition(in: text, at: range.lowerBound),
            end: textPosition(in: text, at: range.upperBound)
        )
    }

    private func textPosition(in text: String, at target: String.Index) -> TextPosition {
        var line = 0
        var column = 0
        var index = text.startIndex

        while index < target {
            if text[index] == "\n" {
                line += 1
                column = 0
            } else {
                column += text[index].unicodeScalars.count
            }
            index = text.index(after: index)
        }

        return TextPosition(line: line, column: column)
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

    private static func requiredBundleDirectory(_ name: String) throws -> URL {
        if let url = Bundle.main.url(forResource: name, withExtension: nil), isDirectory(url) {
            return url
        }

        if let resourceURL = Bundle.main.resourceURL {
            let url = resourceURL.appendingPathComponent(name, isDirectory: true)
            if isDirectory(url) {
                return url
            }
        }

        throw DemoError.missingResource(name)
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
    case missingResource(String)
    case syntaxCompile(String)

    var description: String {
        switch self {
        case let .missingResource(path):
            return "Missing bundled resource: \(path)"
        case let .syntaxCompile(message):
            return message
        }
    }
}
