# SweetLine macOS Swift API

This document describes the Swift Package SDK in `platform/macOS/SweetLine`.

## Overview

- Module: `SweetLine`
- Binding style: Swift API over the SweetLine C API and bundled `SweetLineCoreOSX.xcframework`
- Demo project: `platform/macOS/SweetLineDemo` (SwiftUI/AppKit)
- Package version: `1.2.4`

## Install

For local development, add this folder as a local Swift Package dependency from a macOS app project:

```text
platform/macOS/SweetLine
```

Then import the public SDK module:

```swift
import SweetLine
```

## Native Artifact

The package uses a local binary target:

```swift
.binaryTarget(
    name: "SweetLineCoreOSX",
    path: "Vendor/macOS/SweetLineCoreOSX.xcframework"
)
```

Refresh the bundled artifact from the repository root:

```bash
scripts/build-shared.sh --platform osx
platform/macOS/SweetLine/Scripts/sync-native.sh
```

## Core Types

- `HighlightConfig(showIndex:inlineStyle:)`
- `HighlightEngine`
- `TextAnalyzer`
- `Document`
- `DocumentAnalyzer`
- `TextPosition`, `TextRange`, `TextLineInfo`, `LineRange`
- `TokenStyle`, `TokenSpan`, `LineHighlight`, `DocumentHighlight`
- `DocumentHighlightSlice`
- `IndentGuideLine`, `IndentGuideResult`, `LineScopeState`
- `SweetLineError`, `SyntaxCompileError`

## HighlightEngine

```swift
public final class HighlightEngine {
    public init(config: HighlightConfig = HighlightConfig()) throws

    public func registerStyleName(_ styleName: String, id styleId: Int32) throws
    public func getStyleName(id styleId: Int32) throws -> String?
    public func defineMacro(_ macroName: String) throws
    public func undefineMacro(_ macroName: String) throws

    public func compileSyntax(fromJson syntaxJson: String) throws
    public func compileSyntax(fromFile path: String) throws

    public func createAnalyzer(syntaxName: String) throws -> TextAnalyzer?
    public func createAnalyzer(fileName: String) throws -> TextAnalyzer?
    public func loadDocument(_ document: Document) throws -> DocumentAnalyzer?

    public func close()
}
```

## TextAnalyzer

```swift
public final class TextAnalyzer {
    public func analyzeText(_ text: String) throws -> DocumentHighlight
    public func analyzeLine(_ text: String, info: TextLineInfo) throws -> LineAnalyzeResult
    public func analyzeIndentGuides(_ text: String) throws -> IndentGuideResult
    public func close()
}
```

## DocumentAnalyzer

```swift
public final class DocumentAnalyzer {
    public func analyze() throws -> DocumentHighlight
    public func analyzeLineRange(_ visibleRange: LineRange) throws -> DocumentHighlightSlice
    public func analyzeIncremental(range: TextRange, newText: String) throws -> DocumentHighlight
    public func analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange
    ) throws -> DocumentHighlightSlice
    public func getHighlightSlice(_ visibleRange: LineRange) throws -> DocumentHighlightSlice
    public func analyzeIndentGuides() throws -> IndentGuideResult
    public func close()
}
```

`analyzeLineRange(...)` analyzes enough lines from the current managed document state to satisfy the requested visible range.
`analyzeIncrementalInLineRange(...)` applies a patch and immediately returns the requested slice.
`getHighlightSlice(...)` reads a visible slice from the latest cached result after `analyze()` or `analyzeIncremental(...)`.

## SyntaxCompileError

`SyntaxCompileError` exposes the same compile error code constants as the Java 22 and C# desktop bindings:

```swift
SyntaxCompileError.ok
SyntaxCompileError.jsonPropertyMissed
SyntaxCompileError.jsonPropertyInvalid
SyntaxCompileError.patternInvalid
SyntaxCompileError.stateInvalid
SyntaxCompileError.jsonInvalid
SyntaxCompileError.fileNotExists
SyntaxCompileError.fileInvalid
SyntaxCompileError.importSyntaxNotFound
SyntaxCompileError.stateReferenceNotFound
SyntaxCompileError.inlineStyleReferenceNotFound
```

## Complete Example

```swift
import SweetLine

let sourceCode = "struct Demo { let value = 1 }"

let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
defer { engine.close() }

try engine.registerStyleName("keyword", id: 1)
try engine.registerStyleName("string", id: 2)
try engine.compileSyntax(fromFile: "syntaxes/swift.json")

if let textAnalyzer = try engine.createAnalyzer(fileName: "Demo.swift") {
    let preview = try textAnalyzer.analyzeText(sourceCode)
    let line = try textAnalyzer.analyzeLine(sourceCode, info: TextLineInfo(line: 0))
    let guides = try textAnalyzer.analyzeIndentGuides(sourceCode)
    _ = (preview, line, guides)
    textAnalyzer.close()
}

let document = try Document(uri: "file:///Demo.swift", text: sourceCode)
defer { document.close() }

if let analyzer = try engine.loadDocument(document) {
    let initial = try analyzer.analyze()
    let change = TextRange(
        start: TextPosition(line: 0, column: 7),
        end: TextPosition(line: 0, column: 11)
    )
    let updated = try analyzer.analyzeIncremental(range: change, newText: "Sample")
    let cachedVisible = try analyzer.getHighlightSlice(LineRange(startLine: 0, lineCount: 80))
    let visible = try analyzer.analyzeIncrementalInLineRange(
        range: change,
        newText: "Sample",
        visibleRange: LineRange(startLine: 0, lineCount: 80)
    )
    let indentGuides = try analyzer.analyzeIndentGuides()
    _ = (initial, updated, cachedVisible, visible, indentGuides)
    analyzer.close()
}
```
