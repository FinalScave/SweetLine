# SweetLine for macOS

SweetLine for macOS is a Swift Package SDK over the SweetLine native core.

## Install

For local development, add this folder as a local Swift Package dependency from a macOS app project:

```text
platform/MacOS/SweetLine
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
platform/MacOS/SweetLine/Scripts/sync-native.sh
```

For remote SwiftPM distribution, publish `SweetLineCoreOSX.xcframework.zip` as a release artifact and replace the local binary target with a URL plus checksum.

## Usage

```swift
import SweetLine

let engine = try HighlightEngine(config: HighlightConfig(showIndex: true, inlineStyle: false))
try engine.compileSyntax(fromFile: "/path/to/swift.json")

let analyzer = try engine.createAnalyzer(fileName: "main.swift")
let highlight = try analyzer?.analyzeText("let value = 1")
```

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
Vendor/macOS/SweetLineCoreOSX.xcframework
Tests/SweetLineTests/
```
