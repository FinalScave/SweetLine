# SweetLine KMP

SweetLine KMP is the Kotlin Multiplatform binding for the SweetLine native syntax highlighting engine.

## Features

- Full-text syntax highlighting
- Managed-document incremental highlighting
- Visible-range highlight slice retrieval
- Scope and indent guide analysis
- Bracket pair analysis for rainbow bracket rendering and partner lookup
- Kotlin APIs for Android, iOS, and JVM desktop targets
- Managed-document removal with `HighlightEngine.removeDocument(...)`
- Explicit native resource release with `close()`

## Dependency

```kotlin
implementation("com.qiplat:sweetline-kmp:1.0.0")
```

## Managed Documents

```kotlin
val uri = "Main.kt"
val engine = HighlightEngine(HighlightConfig())
val document = Document(uri, sourceText)
val analyzer = engine.loadDocument(document)

try {
    val highlight = analyzer?.analyze()
} finally {
    analyzer?.close()
    engine.removeDocument(uri)
    document.close()
    engine.close()
}
```

## Resource Management

`HighlightEngine`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer` own native resources. Call `close()` when each object is no longer needed.

For managed documents, close the analyzer, call `HighlightEngine.removeDocument(uri)`, close the document, and finally close the engine.

## Bracket Pair Analysis

`TextAnalyzer.analyzeBracketPairs(...)` and `DocumentAnalyzer.analyzeBracketPairs(...)` return `BracketPairResult`.
Each `BracketToken` includes the token range, nesting depth, kind, match state, and optional partner range.

Use `BracketToken.depth` to pick rainbow bracket colors and `BracketToken.partnerRange` to implement matching bracket navigation.
For editors that only need the viewport, use `DocumentAnalyzer.analyzeBracketPairsInLineRange(...)`.

## Development Project

This directory also contains a Kotlin Multiplatform demo project targeting Android, iOS, and Desktop JVM.

### Build and Run Android Application

To build and run the development Android app, use the run configuration from the IDE toolbar or build it directly from the terminal:

```shell
./gradlew :demo:assembleDebug
```

On Windows:

```shell
.\gradlew.bat :demo:assembleDebug
```

### Build and Run Desktop JVM Application

```shell
./gradlew :demo:jvmRun
```

On Windows:

```shell
.\gradlew.bat :demo:jvmRun
```

### Build and Run iOS Application

Open the `iosApp` directory in Xcode and run the iOS application from there.
