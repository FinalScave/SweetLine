import XCTest
import AppKit
@testable import SweetLineMacOS

final class SweetLineAppleTests: XCTestCase {
    func testSDKVersionIsDefined() {
        XCTAssertFalse(SweetLineMacOSSDK.version.isEmpty)
    }

    func testPublicUITypeNamesFollowCrossPlatformNaming() {
        let theme = HighlightTheme(name: "Default", backgroundColorARGB: 0, textColorARGB: 0, colorMap: [:])
        let renderModel = CodeRenderModel(
            sourceText: "",
            highlight: DocumentHighlight(lines: []),
            indentGuides: IndentGuideResult(guides: [], lineStates: []),
            theme: theme
        )

        _ = CodeView(frame: NSRect.zero)
        _ = CodeLayout.gutterWidth(lineCount: 1, digitWidth: 8)
        _ = renderModel
    }

    func testEngineCanCompileSwiftSyntaxAndAnalyzeText() throws {
        let repositoryRoot = URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
        let syntaxPath = repositoryRoot.appendingPathComponent("syntaxes/swift.json").path

        let engine = try HighlightEngine()
        try engine.compileSyntax(fromFile: syntaxPath)

        let analyzer = try engine.createAnalyzer(named: "swift")
        let result = try analyzer.analyze("let value = 1")

        XCTAssertEqual(result.lines.count, 1)
        XCTAssertFalse(result.lines[0].spans.isEmpty)
    }
}
