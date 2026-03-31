import XCTest
import CoreGraphics
import SweetLineMacOS

final class DemoCodeLayoutTests: XCTestCase {
    func testGutterWidthScalesWithLineCountDigits() {
        let singleDigit = CodeLayout.gutterWidth(lineCount: 9, digitWidth: 8)
        let tripleDigit = CodeLayout.gutterWidth(lineCount: 120, digitWidth: 8)

        XCTAssertLessThan(singleDigit, tripleDigit)
    }

    func testSegmentsIncludePlainTextAroundStyledSpans() {
        let segments = CodeLayout.makeSegments(
            lineText: "let value = 42",
            spans: [TokenSpan(column: 0, length: 3, styleID: 1)]
        )

        XCTAssertEqual(segments.count, 2)
        XCTAssertEqual(segments[0].text, "let")
        XCTAssertEqual(segments[0].styleID, 1)
        XCTAssertEqual(segments[1].text, " value = 42")
        XCTAssertNil(segments[1].styleID)
    }

    func testPreferredContentSizeAccountsForLineCountAndLongestLine() {
        let size = CodeLayout.preferredContentSize(
            lines: ["short", "a much longer line"],
            lineHeight: 18,
            characterWidth: 8,
            gutterWidth: 40
        )

        XCTAssertGreaterThan(size.width, 40)
        XCTAssertGreaterThan(size.height, 30)
    }

    func testPreferredContentSizeUsesSingleGutterContribution() {
        let gutterWidth: CGFloat = 40
        let characterWidth: CGFloat = 8
        let size = CodeLayout.preferredContentSize(
            lines: ["12345"],
            lineHeight: 18,
            characterWidth: characterWidth,
            gutterWidth: gutterWidth
        )

        XCTAssertEqual(size.width, gutterWidth + 8 + 5 * characterWidth + 20)
    }

    func testSegmentsPreserveInlineStyleFromTokenSpan() {
        let inlineStyle = InlineStyle(
            foregroundColor: Int32(bitPattern: 0xFF123456),
            backgroundColor: Int32(bitPattern: 0x00000000),
            fontAttributes: FontAttributes()
        )

        let segments = CodeLayout.makeSegments(
            lineText: "let",
            spans: [TokenSpan(column: 0, length: 3, styleID: nil, inlineStyle: inlineStyle)]
        )

        XCTAssertEqual(segments.count, 1)
        XCTAssertNil(segments[0].styleID)
        XCTAssertEqual(segments[0].inlineStyle, inlineStyle)
    }
}
