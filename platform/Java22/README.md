# SweetLine for Java 22

SweetLine for Java 22 is an FFM binding over the native SweetLine C API. The package bundles native libraries for supported desktop targets and includes a Swing demo.

## Installation

### Maven

```xml
<dependency>
  <groupId>com.qiplat</groupId>
  <artifactId>sweetline-ffm</artifactId>
  <version>1.3.1</version>
</dependency>
```

### Gradle

```groovy
implementation "com.qiplat:sweetline-ffm:1.3.1"
```

## Requirements

- JDK 22
- `--enable-native-access=ALL-UNNAMED` at runtime

## Quick Start

```java
import com.qiplat.sweetline.*;

try (HighlightEngine engine = new HighlightEngine(new HighlightConfig(true, false))) {
    engine.compileSyntaxFromFile("syntaxes/java.json");

    try (TextAnalyzer analyzer = engine.createAnalyzerByFileName("Demo.java")) {
        if (analyzer != null) {
            DocumentHighlight result = analyzer.analyzeText("public class Demo {}");
        }
    }
}
```

## Managed Documents

Use `HighlightEngine.removeDocument(uri)` after closing the document analyzer to remove the document and its cached analyzer from the engine.

```java
String uri = "file:///Demo.java";

try (HighlightEngine engine = new HighlightEngine();
     Document document = new Document(uri, sourceCode)) {
    engine.compileSyntaxFromFile("syntaxes/java.json");

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

`HighlightEngine`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer` implement `AutoCloseable`. Prefer try-with-resources. Each class also provides a `finalize()` fallback for objects that were not closed explicitly, but its execution timing is not guaranteed.

For managed documents, release resources in this order:

- close `DocumentAnalyzer`
- call `HighlightEngine.removeDocument(uri)`
- close `Document`
- close `HighlightEngine`

## Native Loading

Native loading checks the `sweetline.lib.path` system property, common build output directories, bundled JAR resources, and `java.library.path`.

```bash
-Dsweetline.lib.path=/path/to/native/lib/dir
```

## Build and Run

```powershell
cd platform/Java22
.\gradlew.bat :sweetline:build
.\gradlew.bat :demo:run
```

See [CHANGELOG.md](CHANGELOG.md) for release notes.
