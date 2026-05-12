import XCTest
@testable import SweetLine

final class SweetLineTests: XCTestCase {
    func testDefaultHighlightConfig() {
        let config = HighlightConfig()

        XCTAssertFalse(config.showIndex)
        XCTAssertFalse(config.inlineStyle)
    }

    func testSDKVersionMatchesDesktopBindings() {
        XCTAssertEqual(SweetLineSDK.version, "1.2.4")
    }

    func testSyntaxCompileErrorCodeConstants() {
        XCTAssertEqual(SyntaxCompileError.ok, 0)
        XCTAssertEqual(SyntaxCompileError.jsonPropertyMissed, -1)
        XCTAssertEqual(SyntaxCompileError.jsonPropertyInvalid, -2)
        XCTAssertEqual(SyntaxCompileError.patternInvalid, -3)
        XCTAssertEqual(SyntaxCompileError.stateInvalid, -4)
        XCTAssertEqual(SyntaxCompileError.jsonInvalid, -5)
        XCTAssertEqual(SyntaxCompileError.fileNotExists, -6)
        XCTAssertEqual(SyntaxCompileError.fileInvalid, -7)
        XCTAssertEqual(SyntaxCompileError.importSyntaxNotFound, -8)
        XCTAssertEqual(SyntaxCompileError.stateReferenceNotFound, -9)
        XCTAssertEqual(SyntaxCompileError.inlineStyleReferenceNotFound, -10)
    }

    func testNativeEngineCanCreateAndClose() throws {
        let engine = try HighlightEngine()
        engine.close()
    }

    func testCompileSyntaxFromFileCreatesFileAnalyzer() throws {
        let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
        defer { engine.close() }
        try registerStyleNames(engine)

        try engine.compileSyntax(fromFile: try syntaxFileURL().path)

        let analyzer = try XCTUnwrap(engine.createAnalyzer(fileName: "Demo.swift"))
        defer { analyzer.close() }

        let highlight = try analyzer.analyzeText("struct Demo { let value = 1 }")
        XCTAssertFalse(highlight.lines.isEmpty)
    }

    func testDesktopApiParitySurfaceCanAnalyzeSwift() throws {
        let source = """
        struct Demo {
            let value = 1
        }
        """

        let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
        defer { engine.close() }
        try registerStyleNames(engine)
        try engine.defineMacro("WINDOWS")
        try engine.undefineMacro("WINDOWS")

        let syntaxJson = try String(contentsOf: syntaxFileURL(), encoding: .utf8)
        try engine.compileSyntax(fromJson: syntaxJson)
        XCTAssertEqual(try engine.getStyleName(id: 1), "keyword")

        let textAnalyzer = try XCTUnwrap(engine.createAnalyzer(syntaxName: "swift"))
        defer { textAnalyzer.close() }

        let textHighlight = try textAnalyzer.analyzeText(source)
        XCTAssertEqual(textHighlight.lines.count, source.components(separatedBy: "\n").count)

        let lineResult = try textAnalyzer.analyzeLine(
            "    let value = 1",
            info: TextLineInfo(line: 1, startState: 0, startCharOffset: 14)
        )
        XCTAssertGreaterThan(lineResult.charCount, 0)

        let textGuides = try textAnalyzer.analyzeIndentGuides(source)
        XCTAssertNotNil(textGuides)

        let fileAnalyzer = try XCTUnwrap(engine.createAnalyzer(fileName: "Demo.swift"))
        defer { fileAnalyzer.close() }
        XCTAssertFalse(try fileAnalyzer.analyzeText(source).lines.isEmpty)

        let document = try Document(uri: "file:///Demo.swift", text: source)
        defer { document.close() }

        let documentAnalyzer = try XCTUnwrap(engine.loadDocument(document))
        defer { documentAnalyzer.close() }

        let full = try documentAnalyzer.analyze()
        XCTAssertEqual(full.lines.count, textHighlight.lines.count)

        let analyzedSlice = try documentAnalyzer.analyzeLineRange(LineRange(startLine: 0, lineCount: 2))
        XCTAssertEqual(analyzedSlice.startLine, 0)
        XCTAssertFalse(analyzedSlice.lines.isEmpty)

        let change = TextRange(
            start: TextPosition(line: 1, column: 8),
            end: TextPosition(line: 1, column: 13)
        )
        let updated = try documentAnalyzer.analyzeIncremental(range: change, newText: "answer")
        XCTAssertFalse(updated.lines.isEmpty)

        let cachedSlice = try documentAnalyzer.getHighlightSlice(LineRange(startLine: 0, lineCount: 2))
        XCTAssertEqual(cachedSlice.startLine, 0)
        XCTAssertFalse(cachedSlice.lines.isEmpty)

        let visible = try documentAnalyzer.analyzeIncrementalInLineRange(
            range: change,
            newText: "total",
            visibleRange: LineRange(startLine: 0, lineCount: 2)
        )
        XCTAssertEqual(visible.startLine, 0)
        XCTAssertFalse(visible.lines.isEmpty)

        let documentGuides = try documentAnalyzer.analyzeIndentGuides()
        XCTAssertNotNil(documentGuides)
    }

    private func syntaxFileURL() throws -> URL {
        try repoRoot().appendingPathComponent("syntaxes/swift.json")
    }

    private func repoRoot() throws -> URL {
        var url = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
        while url.path != "/" {
            let candidate = url.appendingPathComponent("syntaxes/swift.json")
            if FileManager.default.isReadableFile(atPath: candidate.path) {
                return url
            }
            url.deleteLastPathComponent()
        }
        throw NSError(
            domain: "SweetLineTests",
            code: 1,
            userInfo: [NSLocalizedDescriptionKey: "Unable to find repository root containing syntaxes/swift.json."]
        )
    }

    private func registerStyleNames(_ engine: HighlightEngine) throws {
        try engine.registerStyleName("keyword", id: 1)
        try engine.registerStyleName("string", id: 2)
        try engine.registerStyleName("number", id: 3)
        try engine.registerStyleName("comment", id: 4)
        try engine.registerStyleName("class", id: 5)
        try engine.registerStyleName("method", id: 6)
        try engine.registerStyleName("variable", id: 7)
        try engine.registerStyleName("punctuation", id: 8)
        try engine.registerStyleName("annotation", id: 9)
        try engine.registerStyleName("preprocessor", id: 10)
        try engine.registerStyleName("macro", id: 11)
        try engine.registerStyleName("lifetime", id: 12)
        try engine.registerStyleName("selector", id: 13)
        try engine.registerStyleName("builtin", id: 14)
        try engine.registerStyleName("url", id: 15)
        try engine.registerStyleName("property", id: 16)
    }
}
