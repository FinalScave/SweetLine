import XCTest
@testable import SweetLine

final class SweetLineTests: XCTestCase {
    func testDefaultHighlightConfig() {
        let config = HighlightConfig()

        XCTAssertFalse(config.showIndex)
        XCTAssertFalse(config.inlineStyle)
    }

    func testNativeEngineCanCreateAndClose() throws {
        let engine = try HighlightEngine()
        engine.close()
    }
}
