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
try engine.compileSyntax(fromJson: syntaxJson)

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
Vendor/iOS/SweetLineCoreIOS.xcframework
Tests/SweetLineTests/
```

The demo app should live outside this SDK package, for example `platform/iOS/SweetLineDemo`, and consume this package as a local Swift Package dependency.
