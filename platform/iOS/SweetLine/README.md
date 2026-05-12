# SweetLine for iOS

SweetLine for iOS is a Swift Package SDK over the SweetLine native core.

## Install

For local development, add this folder as a local Swift Package dependency from an iOS app project:

```text
platform/iOS/SweetLine
```

Then import the public SDK module:

```swift
import SweetLine
```

## Native Artifact

The package uses a local binary target:

```swift
.binaryTarget(
    name: "SweetLineCoreIOS",
    path: "Vendor/iOS/SweetLineCoreIOS.xcframework"
)
```

Refresh the bundled artifact from the repository root:

```bash
scripts/build-shared.sh --platform ios
platform/iOS/SweetLine/Scripts/sync-native.sh
```

For remote SwiftPM distribution, publish `SweetLineCoreIOS.xcframework.zip` as a release artifact and replace the local binary target with a URL plus checksum.

## Usage

```swift
import SweetLine

let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
try engine.registerStyleName("keyword", id: 1)
try engine.registerStyleName("string", id: 2)
try engine.compileSyntax(fromJson: syntaxJson)

let analyzer = try engine.createAnalyzer(fileName: "main.swift")
let highlight = try analyzer?.analyzeText("let value = 1")

let document = try Document(uri: "file:///main.swift", text: "let value = 1")
defer { document.close() }

let documentAnalyzer = try engine.loadDocument(document)
let full = try documentAnalyzer?.analyze()
let guides = try documentAnalyzer?.analyzeIndentGuides()
let updated = try documentAnalyzer?.analyzeIncremental(
    range: TextRange(
        start: TextPosition(line: 0, column: 4),
        end: TextPosition(line: 0, column: 9)
    ),
    newText: "answer"
)
let visible = try documentAnalyzer?.getHighlightSlice(LineRange(startLine: 0, lineCount: 80))
```

## Core Types

- `HighlightConfig(showIndex:inlineStyle:)`
- `HighlightEngine`
- `TextAnalyzer`
- `Document`
- `DocumentAnalyzer`
- `TextPosition`, `TextRange`, `TextLineInfo`, `LineRange`
- `TokenSpan`, `LineHighlight`, `DocumentHighlight`, `DocumentHighlightSlice`
- `IndentGuideLine`, `IndentGuideResult`, `LineScopeState`
- `SyntaxCompileError`

## Layout

```text
Package.swift
Sources/SweetLine/
  SweetLine.swift
  SweetLineNative.swift
  NativeBufferParser.swift
  HighlightEngine.swift
  TextAnalyzer.swift
  Document.swift
  DocumentAnalyzer.swift
  Models.swift
  Errors.swift
Vendor/iOS/SweetLineCoreIOS.xcframework
Tests/SweetLineTests/
```

The demo app lives outside this SDK package at `platform/iOS/SweetLineDemo` and consumes this package as a local Swift Package dependency.
