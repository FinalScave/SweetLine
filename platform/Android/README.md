# SweetLine for Android

SweetLine for Android exposes the native highlighting engine through a Java/JNI API. The project contains the core `sweetline` library and an optional Markwon integration module.

## Installation

```groovy
dependencies {
    implementation "com.qiplat:sweetline:1.3.1"
    implementation "com.qiplat:sweetline-markwon:1.3.1"
}
```

The Markwon dependency is optional. Source consumers can include the `:sweetline` or `:markwon-plugin` module directly.

## Features

- Full-text and single-line highlighting
- Managed-document incremental highlighting
- Visible-range highlight slices
- Indent guide and bracket pair analysis
- Direct `Spannable` output for Android text widgets
- File-name-based syntax routing
- Explicit native resource management through `AutoCloseable`

## Quick Start

```java
import com.qiplat.sweetline.*;

HighlightConfig config = new HighlightConfig();
config.showIndex = true;

try (HighlightEngine engine = new HighlightEngine(config);
     SyntaxRule syntax = engine.compileSyntaxFromJson(syntaxJson)) {
    engine.registerStyleName("keyword", 1);

    try (TextAnalyzer analyzer = engine.createAnalyzerByFileName("MainActivity.java")) {
        if (analyzer != null) {
            DocumentHighlight result = analyzer.analyzeText(sourceCode);
        }
    }
}
```

## Managed Documents

`HighlightEngine.removeDocument(uri)` removes a managed document and its cached analyzer from the engine. Close the document analyzer first, remove the document from the engine, and then close the document.

```java
String uri = "file:///MainActivity.java";

try (HighlightEngine engine = new HighlightEngine(config);
     Document document = new Document(uri, sourceCode)) {
    engine.compileSyntaxFromJson(syntaxJson).close();

    DocumentAnalyzer loaded = engine.loadDocument(document);
    if (loaded != null) {
        try (DocumentAnalyzer analyzer = loaded) {
            DocumentHighlight initial = analyzer.analyze();
            DocumentHighlightSlice visible = analyzer.getHighlightSlice(
                    new LineRange(0, 80));
        }
    }

    engine.removeDocument(uri);
}
```

## Resource Management

`HighlightEngine`, `SyntaxRule`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer` implement `AutoCloseable`. Prefer try-with-resources and release native resources deterministically. Each class also provides a `finalize()` fallback for objects that were not closed explicitly, but its execution timing is not guaranteed.

For managed documents, use this order:

- close `DocumentAnalyzer`
- call `HighlightEngine.removeDocument(uri)`
- close `Document`
- close `HighlightEngine`

Calling `close()` more than once is safe.

## Build

```powershell
cd platform/Android
.\gradlew.bat :sweetline:assembleRelease
```

The Android library builds `arm64-v8a` and `x86_64` native binaries.

See [CHANGELOG.md](CHANGELOG.md) for release notes.
