import XCTest
@testable import SweetLineMacOS

final class CodeSegmentStyleTests: XCTestCase {
    func testSegmentColorPrefersInlineForegroundWhenStyleIDIsMissing() {
        let inlineStyle = InlineStyle(
            foregroundColor: Int32(bitPattern: 0xFF12AB34),
            backgroundColor: Int32(bitPattern: 0x00000000),
            fontAttributes: FontAttributes()
        )
        let segment = CodeSegment(text: "let", styleID: nil, inlineStyle: inlineStyle)
        let theme = HighlightTheme(
            name: "theme",
            backgroundColorARGB: Int32(bitPattern: 0xFF000000),
            textColorARGB: Int32(bitPattern: 0xFFFFFFFF),
            colorMap: [1: Int32(bitPattern: 0xFFAA0000)]
        )

        XCTAssertEqual(segment.foregroundColorARGB(theme: theme), inlineStyle.foregroundColor)
    }

    func testSegmentColorPrefersInlineForegroundOverThemeStyle() {
        let inlineStyle = InlineStyle(
            foregroundColor: Int32(bitPattern: 0xFF33CC55),
            backgroundColor: Int32(bitPattern: 0x00000000),
            fontAttributes: FontAttributes()
        )
        let theme = HighlightTheme(
            name: "theme",
            backgroundColorARGB: Int32(bitPattern: 0xFF000000),
            textColorARGB: Int32(bitPattern: 0xFFFFFFFF),
            colorMap: [7: Int32(bitPattern: 0xFFAA0000)]
        )
        let segment = CodeSegment(text: "kw", styleID: 7, inlineStyle: inlineStyle)

        XCTAssertEqual(segment.foregroundColorARGB(theme: theme), inlineStyle.foregroundColor)
    }

    func testSegmentColorFallsBackToThemeStyleAndDefaultTextColor() {
        let theme = HighlightTheme(
            name: "theme",
            backgroundColorARGB: Int32(bitPattern: 0xFF000000),
            textColorARGB: Int32(bitPattern: 0xFFFFFFFF),
            colorMap: [7: Int32(bitPattern: 0xFFAA0000)]
        )

        XCTAssertEqual(CodeSegment(text: "kw", styleID: 7, inlineStyle: nil).foregroundColorARGB(theme: theme), Int32(bitPattern: 0xFFAA0000))
        XCTAssertEqual(CodeSegment(text: "plain", styleID: nil, inlineStyle: nil).foregroundColorARGB(theme: theme), Int32(bitPattern: 0xFFFFFFFF))
    }
}
