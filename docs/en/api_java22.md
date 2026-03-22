# SweetLine Java 22 (FFM) API

This document describes the Java 22 FFM wrapper in `platform/Java22/sweetline`.

---

## Overview

- Package: `com.qiplat.sweetline`
- Binding style: Java 22 Foreign Function & Memory (FFM) over SweetLine C API
- Demo project: `platform/Java22/demo` (Swing)

---

## Dependency

### Maven

```xml
<dependency>
  <groupId>com.qiplat</groupId>
  <artifactId>sweetline-ffm</artifactId>
  <version>1.1.0</version>
</dependency>
```

### Gradle

```groovy
// build.gradle
implementation 'com.qiplat:sweetline-ffm:1.1.0'
```

---

## Requirements

- JDK 22
- Runtime flags:
  - `--enable-preview`
  - `--enable-native-access=ALL-UNNAMED`
- Native library (`sweetline`) must be loadable (see [Native Loading](#native-loading))

---

## Build and Run

```bash
cd platform/Java22
./gradlew :sweetline:build
./gradlew :demo:run
```

---

## Core Types

- `HighlightConfig(boolean showIndex, boolean inlineStyle)`
- `HighlightEngine`
- `Document` (`AutoCloseable`)
- `TextAnalyzer` (`AutoCloseable`)
- `DocumentAnalyzer` (`AutoCloseable`)
- `TextPosition`, `TextRange`, `TextLineInfo`
- `TokenSpan`, `LineHighlight`, `DocumentHighlight`
- `LineRange`, `DocumentHighlightSlice`
- `IndentGuideLine`, `IndentGuideResult`, `LineScopeState`
- `SyntaxCompileError`

Note:
- `HighlightConfig` in Java 22 currently includes `showIndex` and `inlineStyle`.

---

## HighlightEngine

```java
public class HighlightEngine implements AutoCloseable {
    public HighlightEngine(HighlightConfig config);
    public HighlightEngine();

    public void registerStyleName(String styleName, int styleId);
    public String getStyleName(int styleId);
    public void defineMacro(String macroName);
    public void undefineMacro(String macroName);

    public void compileSyntaxFromJson(String syntaxJson) throws SyntaxCompileError;
    public void compileSyntaxFromFile(String path) throws SyntaxCompileError;

    public TextAnalyzer createAnalyzerByName(String syntaxName);
    public TextAnalyzer createAnalyzerByExtension(String extension);
    public DocumentAnalyzer loadDocument(Document document);

    public void close();
}
```

---

## TextAnalyzer

```java
public class TextAnalyzer implements AutoCloseable {
    public DocumentHighlight analyzeText(String text);
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info);
    public IndentGuideResult analyzeIndentGuides(String text);
    public void close();
}
```

---

## DocumentAnalyzer

```java
public class DocumentAnalyzer implements AutoCloseable {
    public DocumentHighlight analyze();
    public DocumentHighlight analyzeIncremental(TextRange range, String newText);
    public DocumentHighlightSlice analyzeIncrementalInLineRange(
            TextRange range, String newText, LineRange visibleRange);
    public IndentGuideResult analyzeIndentGuides();
    public void close();
}
```

---

## Native Loading

`SweetLineNative` resolves native library in this order:

1. System property `sweetline.lib.path`
2. Common source/build output directories
3. JAR resource extraction fallback (`NativeLibraryExtractor.extractToDefaultDir()`)
4. `java.library.path` / `System.loadLibrary`

You can explicitly set:

```bash
-Dsweetline.lib.path=/path/to/native/lib/dir
```

JAR packaging scenario:

```java
import com.qiplat.sweetline.NativeLibraryExtractor;
import java.nio.file.Path;

Path libPath = NativeLibraryExtractor.extractToDefaultDir();
```

---

## Complete Example

```java
import com.qiplat.sweetline.*;

String sourceCode = "public class Demo {}";

try (HighlightEngine engine = new HighlightEngine(new HighlightConfig(true, false))) {
    engine.registerStyleName("keyword", 1);
    engine.registerStyleName("string", 2);
    engine.compileSyntaxFromFile("syntaxes/java.json");

    try (TextAnalyzer textAnalyzer = engine.createAnalyzerByName("java")) {
        if (textAnalyzer != null) {
            DocumentHighlight full = textAnalyzer.analyzeText(sourceCode);
        }
    }

    try (Document document = new Document("file:///Demo.java", sourceCode)) {
        DocumentAnalyzer analyzer = engine.loadDocument(document);
        if (analyzer != null) {
            try (analyzer) {
                DocumentHighlight initial = analyzer.analyze();

                TextRange change = new TextRange(
                        new TextPosition(0, 13),
                        new TextPosition(0, 17));
                DocumentHighlight updated = analyzer.analyzeIncremental(change, "Sample");

                DocumentHighlightSlice visible = analyzer.analyzeIncrementalInLineRange(
                        change,
                        "Sample",
                        new LineRange(0, 80));

                IndentGuideResult guides = analyzer.analyzeIndentGuides();
            }
        }
    }
}
```
