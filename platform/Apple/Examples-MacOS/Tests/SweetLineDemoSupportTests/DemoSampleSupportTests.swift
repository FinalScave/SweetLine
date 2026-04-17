import XCTest
@testable import SweetLineDemoSupport

final class DemoSampleSupportTests: XCTestCase {
    func testDefaultSwiftDemoAnalysisReturnsNonEmptySummary() throws {
        let summary = try DemoSampleSupport.runDefaultSwiftDemoAnalysis()

        XCTAssertEqual(summary.sampleName, "example.swift")
        XCTAssertGreaterThan(summary.lineCount, 0)
        XCTAssertGreaterThan(summary.highlightedLineCount, 0)
        XCTAssertGreaterThan(summary.spanCount, 0)
        XCTAssertFalse(summary.firstLinePreview.isEmpty)
    }

    func testDefaultRenderModelContainsHighlightDataAndTheme() throws {
        let renderModel = try DemoSampleSupport.makeDefaultRenderModel()

        XCTAssertEqual(renderModel.sample.fileName, "example.swift")
        XCTAssertEqual(renderModel.selectedTheme.name, "SweetLine Dark")
        XCTAssertEqual(renderModel.availableThemes.count, 7)
        XCTAssertFalse(renderModel.sourceText.isEmpty)
        XCTAssertGreaterThan(renderModel.highlight.lines.count, 0)
        XCTAssertGreaterThan(renderModel.indentGuides.guides.count, 0)
        XCTAssertGreaterThan(renderModel.summary.spanCount, 0)
        XCTAssertGreaterThanOrEqual(renderModel.status.compileMicroseconds, 0)
        XCTAssertGreaterThanOrEqual(renderModel.status.analyzeMicroseconds, 0)
        XCTAssertGreaterThan(renderModel.status.lineCount, 0)
        XCTAssertEqual(renderModel.status.themeName, renderModel.selectedTheme.name)
    }

    func testBuiltinThemesMatchSharedParitySet() {
        let themes = DemoSampleSupport.builtinThemes

        XCTAssertEqual(themes.map(\.name), [
            "SweetLine Dark",
            "Monokai",
            "Dracula",
            "One Dark",
            "Solarized Dark",
            "Nord",
            "GitHub Dark",
        ])
        XCTAssertEqual(themes.first?.color(for: 1), argb(0xFF569CD6))
        XCTAssertEqual(themes.first?.color(for: 999), themes.first?.textColorARGB)
    }

    func testDefaultLoadStateIsReady() throws {
        let state = try DemoSampleSupport.loadDefaultState()

        guard case let .ready(model) = state else {
            return XCTFail("Expected ready state")
        }

        XCTAssertEqual(model.sample.fileName, "example.swift")
    }

    func testSelectingThemeReusesHighlightAndIndentGuideData() throws {
        let model = try DemoSampleSupport.makeDefaultRenderModel()

        let updated = DemoSampleSupport.selectTheme(id: "monokai", in: model)

        XCTAssertEqual(updated.selectedTheme.id, "monokai")
        XCTAssertEqual(updated.highlight, model.highlight)
        XCTAssertEqual(updated.indentGuides, model.indentGuides)
        XCTAssertEqual(updated.status.compileMicroseconds, model.status.compileMicroseconds)
        XCTAssertEqual(updated.status.analyzeMicroseconds, model.status.analyzeMicroseconds)
    }

    func testLoadStateReportsFailureForMissingSampleAsset() {
        let missingSample = DemoSample(fileName: "missing.swift")
        let state = DemoSampleSupport.makeLoadState(sample: missingSample)

        guard case let .failed(message, sampleName, themes) = state else {
            return XCTFail("Expected failed state")
        }

        XCTAssertEqual(sampleName, "missing.swift")
        XCTAssertFalse(message.isEmpty)
        XCTAssertEqual(themes.map { $0.id }, DemoSampleSupport.builtinThemes.map { $0.id })
    }

    func testLoadStateSupportsNonSwiftSampleWhenCoreRoutesByRealFileName() {
        let javaSample = DemoSample(fileName: "example.java")
        let state = DemoSampleSupport.makeLoadState(sample: javaSample)

        guard case let .ready(model) = state else {
            return XCTFail("Expected ready state for Java sample")
        }

        XCTAssertEqual(model.sample.fileName, "example.java")
        XCTAssertGreaterThan(model.highlight.lines.count, 0)
        XCTAssertGreaterThan(model.summary.spanCount, 0)
    }
}

private func argb(_ value: UInt32) -> Int32 {
    Int32(bitPattern: value)
}
