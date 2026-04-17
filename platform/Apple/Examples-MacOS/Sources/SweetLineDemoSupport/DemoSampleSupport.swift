import Foundation
import SweetLineMacOS

public struct DemoSample: Equatable {
    public let fileName: String

    public init(fileName: String) {
        self.fileName = fileName
    }
}

public struct DemoTheme: Equatable {
    public let id: String
    public let name: String
    public let backgroundColorARGB: Int32
    public let textColorARGB: Int32
    public let colorMap: [Int32: Int32]

    public init(
        id: String,
        name: String,
        backgroundColorARGB: Int32,
        textColorARGB: Int32,
        colorMap: [Int32: Int32]
    ) {
        self.id = id
        self.name = name
        self.backgroundColorARGB = backgroundColorARGB
        self.textColorARGB = textColorARGB
        self.colorMap = colorMap
    }

    public func color(for styleID: Int32) -> Int32 {
        colorMap[styleID] ?? textColorARGB
    }
}

public struct DemoAnalysisSummary: Equatable {
    public let sampleName: String
    public let lineCount: Int
    public let highlightedLineCount: Int
    public let spanCount: Int
    public let firstLinePreview: String

    public init(
        sampleName: String,
        lineCount: Int,
        highlightedLineCount: Int,
        spanCount: Int,
        firstLinePreview: String
    ) {
        self.sampleName = sampleName
        self.lineCount = lineCount
        self.highlightedLineCount = highlightedLineCount
        self.spanCount = spanCount
        self.firstLinePreview = firstLinePreview
    }
}

public struct DemoStatusMetrics: Equatable {
    public let compileMicroseconds: Int
    public let analyzeMicroseconds: Int
    public let lineCount: Int
    public let themeName: String

    public init(compileMicroseconds: Int, analyzeMicroseconds: Int, lineCount: Int, themeName: String) {
        self.compileMicroseconds = compileMicroseconds
        self.analyzeMicroseconds = analyzeMicroseconds
        self.lineCount = lineCount
        self.themeName = themeName
    }
}

public struct DemoRenderModel: Equatable {
    public let sample: DemoSample
    public let availableThemes: [DemoTheme]
    public let selectedTheme: DemoTheme
    public let sourceText: String
    public let highlight: DocumentHighlight
    public let indentGuides: IndentGuideResult
    public let summary: DemoAnalysisSummary
    public let status: DemoStatusMetrics

    public init(
        sample: DemoSample,
        availableThemes: [DemoTheme],
        selectedTheme: DemoTheme,
        sourceText: String,
        highlight: DocumentHighlight,
        indentGuides: IndentGuideResult,
        summary: DemoAnalysisSummary,
        status: DemoStatusMetrics
    ) {
        self.sample = sample
        self.availableThemes = availableThemes
        self.selectedTheme = selectedTheme
        self.sourceText = sourceText
        self.highlight = highlight
        self.indentGuides = indentGuides
        self.summary = summary
        self.status = status
    }
}

public enum DemoLoadState: Equatable {
    case ready(DemoRenderModel)
    case failed(message: String, sampleName: String, availableThemes: [DemoTheme])
}

public enum DemoSampleSupport {
    private struct DemoEngineBundle {
        let engine: HighlightEngine
        let compileMicroseconds: Int
    }

    private static let styleKeyword: Int32 = 1
    private static let styleString: Int32 = 2
    private static let styleNumber: Int32 = 3
    private static let styleComment: Int32 = 4
    private static let styleClass: Int32 = 5
    private static let styleMethod: Int32 = 6
    private static let styleVariable: Int32 = 7
    private static let stylePunctuation: Int32 = 8
    private static let styleAnnotation: Int32 = 9
    private static let stylePreprocessor: Int32 = 10
    private static let styleMacro: Int32 = 11
    private static let styleLifetime: Int32 = 12
    private static let styleSelector: Int32 = 13
    private static let styleBuiltin: Int32 = 14
    private static let styleUrl: Int32 = 15
    private static let styleProperty: Int32 = 16
    private static let builtinSampleFileNames: [String] = [
        "example.swift",
        "example.css",
        "example.scss",
        "example.less",
        "example.jsonc",
        "example.json5",
        "example.cmake",
        "example.dockerfile",
        "example.mk",
        "example.properties",
        "example.env",
        "example.proto",
        "example.graphql",
        "example.nginx",
        "example.gitignore",
        "example.diff",
        "example.rb",
        "example.hcl",
        "example.tf",
        "example.vue",
        "example.svelte",
    ]
    private static let excludedSyntaxFileNames: Set<String> = [
        "yaml(non zero width).json",
    ]
    private static let sharedEngineBundleResult: Result<DemoEngineBundle, Error> = {
        do {
            return .success(try makeSharedEngineBundle())
        } catch {
            return .failure(error)
        }
    }()

    public static let placeholderMessage = "SweetLine Apple demo skeleton is ready for integration."

    public static let builtinThemes: [DemoTheme] = [
        makeTheme(
            id: "sweetline-dark",
            name: "SweetLine Dark",
            background: 0xFF1E1E1E,
            text: 0xFFD4D4D4,
            colors: [
                styleKeyword: 0xFF569CD6,
                styleString: 0xFFBD63C5,
                styleNumber: 0xFFE4FAD5,
                styleComment: 0xFF60AE6F,
                styleClass: 0xFF4EC9B0,
                styleMethod: 0xFF9CDCFE,
                styleVariable: 0xFF9B9BC8,
                stylePunctuation: 0xFFD69D85,
                styleAnnotation: 0xFFFFFD9B,
                stylePreprocessor: 0xFF569CD6,
                styleMacro: 0xFF9B9BC8,
                styleLifetime: 0xFF4EC9B0,
                styleSelector: 0xFF4EC9B0,
                styleBuiltin: 0xFF569CD6,
                styleUrl: 0xFF4FC1FF,
                styleProperty: 0xFF9CDCFE,
            ]
        ),
        makeTheme(
            id: "monokai",
            name: "Monokai",
            background: 0xFF272822,
            text: 0xFFF8F8F2,
            colors: [
                styleKeyword: 0xFFF92672,
                styleString: 0xFFE6DB74,
                styleNumber: 0xFFAE81FF,
                styleComment: 0xFF75715E,
                styleClass: 0xFFA6E22E,
                styleMethod: 0xFFA6E22E,
                styleVariable: 0xFFF8F8F2,
                stylePunctuation: 0xFFF8F8F2,
                styleAnnotation: 0xFFE6DB74,
                stylePreprocessor: 0xFFF92672,
                styleMacro: 0xFFAE81FF,
                styleLifetime: 0xFFFD971F,
                styleSelector: 0xFFA6E22E,
                styleBuiltin: 0xFF66D9EF,
                styleUrl: 0xFF66D9EF,
                styleProperty: 0xFFA6E22E,
            ]
        ),
        makeTheme(
            id: "dracula",
            name: "Dracula",
            background: 0xFF282A36,
            text: 0xFFF8F8F2,
            colors: [
                styleKeyword: 0xFFFF79C6,
                styleString: 0xFFF1FA8C,
                styleNumber: 0xFFBD93F9,
                styleComment: 0xFF6272A4,
                styleClass: 0xFF8BE9FD,
                styleMethod: 0xFF50FA7B,
                styleVariable: 0xFFF8F8F2,
                stylePunctuation: 0xFFF8F8F2,
                styleAnnotation: 0xFFFFB86C,
                stylePreprocessor: 0xFFFF79C6,
                styleMacro: 0xFFBD93F9,
                styleLifetime: 0xFFFFB86C,
                styleSelector: 0xFF50FA7B,
                styleBuiltin: 0xFF8BE9FD,
                styleUrl: 0xFF8BE9FD,
                styleProperty: 0xFF50FA7B,
            ]
        ),
        makeTheme(
            id: "one-dark",
            name: "One Dark",
            background: 0xFF282C34,
            text: 0xFFABB2BF,
            colors: [
                styleKeyword: 0xFFC678DD,
                styleString: 0xFF98C379,
                styleNumber: 0xFFD19A66,
                styleComment: 0xFF5C6370,
                styleClass: 0xFFE5C07B,
                styleMethod: 0xFF61AFEF,
                styleVariable: 0xFFE06C75,
                stylePunctuation: 0xFFABB2BF,
                styleAnnotation: 0xFFE5C07B,
                stylePreprocessor: 0xFFC678DD,
                styleMacro: 0xFFD19A66,
                styleLifetime: 0xFF56B6C2,
                styleSelector: 0xFFE5C07B,
                styleBuiltin: 0xFF56B6C2,
                styleUrl: 0xFF61AFEF,
                styleProperty: 0xFF61AFEF,
            ]
        ),
        makeTheme(
            id: "solarized-dark",
            name: "Solarized Dark",
            background: 0xFF002B36,
            text: 0xFF839496,
            colors: [
                styleKeyword: 0xFF859900,
                styleString: 0xFF2AA198,
                styleNumber: 0xFFD33682,
                styleComment: 0xFF586E75,
                styleClass: 0xFFB58900,
                styleMethod: 0xFF268BD2,
                styleVariable: 0xFFCB4B16,
                stylePunctuation: 0xFF839496,
                styleAnnotation: 0xFFB58900,
                stylePreprocessor: 0xFF859900,
                styleMacro: 0xFFCB4B16,
                styleLifetime: 0xFFD33682,
                styleSelector: 0xFF268BD2,
                styleBuiltin: 0xFF268BD2,
                styleUrl: 0xFF268BD2,
                styleProperty: 0xFF268BD2,
            ]
        ),
        makeTheme(
            id: "nord",
            name: "Nord",
            background: 0xFF2E3440,
            text: 0xFFD8DEE9,
            colors: [
                styleKeyword: 0xFF81A1C1,
                styleString: 0xFFA3BE8C,
                styleNumber: 0xFFB48EAD,
                styleComment: 0xFF616E88,
                styleClass: 0xFF8FBCBB,
                styleMethod: 0xFF88C0D0,
                styleVariable: 0xFFD8DEE9,
                stylePunctuation: 0xFFECEFF4,
                styleAnnotation: 0xFFEBCB8B,
                stylePreprocessor: 0xFF81A1C1,
                styleMacro: 0xFFB48EAD,
                styleLifetime: 0xFFEBCB8B,
                styleSelector: 0xFF8FBCBB,
                styleBuiltin: 0xFF5E81AC,
                styleUrl: 0xFF88C0D0,
                styleProperty: 0xFF88C0D0,
            ]
        ),
        makeTheme(
            id: "github-dark",
            name: "GitHub Dark",
            background: 0xFF0D1117,
            text: 0xFFC9D1D9,
            colors: [
                styleKeyword: 0xFFFF7B72,
                styleString: 0xFFA5D6FF,
                styleNumber: 0xFF79C0FF,
                styleComment: 0xFF8B949E,
                styleClass: 0xFFFFA657,
                styleMethod: 0xFFD2A8FF,
                styleVariable: 0xFFFFA657,
                stylePunctuation: 0xFFC9D1D9,
                styleAnnotation: 0xFFFFA657,
                stylePreprocessor: 0xFFFF7B72,
                styleMacro: 0xFF79C0FF,
                styleLifetime: 0xFFFFA657,
                styleSelector: 0xFF7EE787,
                styleBuiltin: 0xFF79C0FF,
                styleUrl: 0xFF79C0FF,
                styleProperty: 0xFF79C0FF,
            ]
        ),
    ]

    public static let builtinSamples: [DemoSample] = builtinSampleFileNames.map(DemoSample.init(fileName:))

    public static func defaultConfig() -> HighlightConfig {
        HighlightConfig(showIndex: true, inlineStyle: false)
    }

    public static func runDefaultSwiftDemoAnalysis() throws -> DemoAnalysisSummary {
        try makeDefaultRenderModel().summary
    }

    public static func loadDefaultState() throws -> DemoLoadState {
        let state = makeLoadState(sample: builtinSamples[0])
        if case let .failed(message, _, _) = state {
            throw NSError(domain: "SweetLineDemoSupport", code: 1, userInfo: [NSLocalizedDescriptionKey: message])
        }
        return state
    }

    public static func makeLoadState(sample: DemoSample) -> DemoLoadState {
        do {
            return .ready(try makeRenderModel(sample: sample, selectedTheme: builtinThemes[0]))
        } catch {
            return .failed(message: String(describing: error), sampleName: sample.fileName, availableThemes: builtinThemes)
        }
    }

    public static func selectTheme(id: String, in model: DemoRenderModel) -> DemoRenderModel {
        let nextTheme = model.availableThemes.first { $0.id == id } ?? model.selectedTheme
        let nextStatus = DemoStatusMetrics(
            compileMicroseconds: model.status.compileMicroseconds,
            analyzeMicroseconds: model.status.analyzeMicroseconds,
            lineCount: model.status.lineCount,
            themeName: nextTheme.name
        )

        return DemoRenderModel(
            sample: model.sample,
            availableThemes: model.availableThemes,
            selectedTheme: nextTheme,
            sourceText: model.sourceText,
            highlight: model.highlight,
            indentGuides: model.indentGuides,
            summary: model.summary,
            status: nextStatus
        )
    }

    public static func makeDefaultRenderModel() throws -> DemoRenderModel {
        try makeRenderModel(sample: builtinSamples[0], selectedTheme: builtinThemes[0])
    }

    private static func makeRenderModel(sample: DemoSample, selectedTheme: DemoTheme) throws -> DemoRenderModel {
        let repositoryRoot = resolveRepositoryRoot()
        let sampleURL = repositoryRoot.appendingPathComponent("tests/files/\(sample.fileName)")
        let sampleText = try String(contentsOf: sampleURL, encoding: .utf8)
        let engineBundle = try sharedEngineBundle()
        let document = try Document(uri: sample.fileName, text: sampleText)
        let analyzer = try engineBundle.engine.loadDocument(document)
        let analyzeStart = DispatchTime.now().uptimeNanoseconds
        let highlight = try analyzer.analyze()
        let indentGuides = try analyzer.analyzeIndentGuides()
        let analyzeMicroseconds = elapsedMicroseconds(since: analyzeStart)

        let summary = makeSummary(sample: sample, sampleText: sampleText, highlight: highlight)
        let status = DemoStatusMetrics(
            compileMicroseconds: engineBundle.compileMicroseconds,
            analyzeMicroseconds: analyzeMicroseconds,
            lineCount: summary.lineCount,
            themeName: selectedTheme.name
        )

        return DemoRenderModel(
            sample: sample,
            availableThemes: builtinThemes,
            selectedTheme: selectedTheme,
            sourceText: sampleText,
            highlight: highlight,
            indentGuides: indentGuides,
            summary: summary,
            status: status
        )
    }

    private static func sharedEngineBundle() throws -> DemoEngineBundle {
        try sharedEngineBundleResult.get()
    }

    private static func makeSharedEngineBundle() throws -> DemoEngineBundle {
        let repositoryRoot = resolveRepositoryRoot()
        let syntaxURLs = try listCommonSyntaxURLs(repositoryRoot: repositoryRoot)
        let engine = try HighlightEngine(config: defaultConfig())
        try registerStyleNames(on: engine)

        let compileStart = DispatchTime.now().uptimeNanoseconds
        try compileCommonSyntaxes(into: engine, syntaxURLs: syntaxURLs)
        let compileMicroseconds = elapsedMicroseconds(since: compileStart)

        return DemoEngineBundle(engine: engine, compileMicroseconds: compileMicroseconds)
    }

    private static func listCommonSyntaxURLs(repositoryRoot: URL) throws -> [URL] {
        let syntaxesURL = repositoryRoot.appendingPathComponent("syntaxes", isDirectory: true)
        let syntaxURLs = try FileManager.default.contentsOfDirectory(
            at: syntaxesURL,
            includingPropertiesForKeys: nil,
            options: [.skipsHiddenFiles]
        )

        return syntaxURLs
            .filter { $0.pathExtension == "json" }
            .filter { !$0.lastPathComponent.hasSuffix("-inlineStyle.json") }
            .filter { !excludedSyntaxFileNames.contains($0.lastPathComponent) }
            .sorted { $0.lastPathComponent.localizedStandardCompare($1.lastPathComponent) == .orderedAscending }
    }

    private static func compileCommonSyntaxes(into engine: HighlightEngine, syntaxURLs: [URL]) throws {
        var pending = syntaxURLs

        while !pending.isEmpty {
            var nextPending: [URL] = []
            var madeProgress = false

            for syntaxURL in pending {
                do {
                    try engine.compileSyntax(fromFile: syntaxURL.path)
                    madeProgress = true
                } catch let error as SyntaxCompileError {
                    if error.errorCode == SyntaxCompileError.errImportSyntaxNotFound {
                        nextPending.append(syntaxURL)
                        continue
                    }
                    throw error
                }
            }

            if !madeProgress {
                let fileNames = nextPending.map(\.lastPathComponent).joined(separator: ", ")
                throw NSError(
                    domain: "SweetLineDemoSupport",
                    code: SyntaxCompileError.errImportSyntaxNotFound,
                    userInfo: [NSLocalizedDescriptionKey: "Unresolved syntax dependencies: \(fileNames)"]
                )
            }

            pending = nextPending
        }
    }

    private static func registerStyleNames(on engine: HighlightEngine) throws {
        try engine.registerStyleName("keyword", styleID: styleKeyword)
        try engine.registerStyleName("string", styleID: styleString)
        try engine.registerStyleName("number", styleID: styleNumber)
        try engine.registerStyleName("comment", styleID: styleComment)
        try engine.registerStyleName("class", styleID: styleClass)
        try engine.registerStyleName("method", styleID: styleMethod)
        try engine.registerStyleName("variable", styleID: styleVariable)
        try engine.registerStyleName("punctuation", styleID: stylePunctuation)
        try engine.registerStyleName("annotation", styleID: styleAnnotation)
        try engine.registerStyleName("preprocessor", styleID: stylePreprocessor)
        try engine.registerStyleName("macro", styleID: styleMacro)
        try engine.registerStyleName("lifetime", styleID: styleLifetime)
        try engine.registerStyleName("selector", styleID: styleSelector)
        try engine.registerStyleName("builtin", styleID: styleBuiltin)
        try engine.registerStyleName("url", styleID: styleUrl)
        try engine.registerStyleName("property", styleID: styleProperty)
    }

    private static func resolveRepositoryRoot() -> URL {
        URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
    }

    private static func makeSummary(sample: DemoSample, sampleText: String, highlight: DocumentHighlight) -> DemoAnalysisSummary {
        let lineCount = highlight.lines.count
        let highlightedLineCount = highlight.lines.filter { !$0.spans.isEmpty }.count
        let spanCount = highlight.lines.reduce(into: 0) { partial, line in
            partial += line.spans.count
        }
        let firstLinePreview = sampleText
            .split(whereSeparator: \.isNewline)
            .first
            .map(String.init) ?? ""

        return DemoAnalysisSummary(
            sampleName: sample.fileName,
            lineCount: lineCount,
            highlightedLineCount: highlightedLineCount,
            spanCount: spanCount,
            firstLinePreview: firstLinePreview
        )
    }

    private static func elapsedMicroseconds(since startNanoseconds: UInt64) -> Int {
        Int((DispatchTime.now().uptimeNanoseconds - startNanoseconds) / 1_000)
    }

    private static func makeTheme(
        id: String,
        name: String,
        background: UInt32,
        text: UInt32,
        colors: [Int32: UInt32]
    ) -> DemoTheme {
        DemoTheme(
            id: id,
            name: name,
            backgroundColorARGB: argb(background),
            textColorARGB: argb(text),
            colorMap: colors.mapValues(argb)
        )
    }

    private static func argb(_ value: UInt32) -> Int32 {
        Int32(bitPattern: value)
    }
}
