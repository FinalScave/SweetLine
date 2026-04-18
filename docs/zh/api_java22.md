# SweetLine Java 22（FFM）API

本文档说明 `platform/Java22/sweetline` 中的 Java 22 FFM 封装。

---

## 概览

- 包名：`com.qiplat.sweetline`
- 绑定方式：基于 Java 22 Foreign Function & Memory（FFM）调用 SweetLine C API
- Demo 工程：`platform/Java22/demo`（Swing）

---

## 依赖引入

### Maven

```xml
<dependency>
  <groupId>com.qiplat</groupId>
  <artifactId>sweetline-ffm</artifactId>
  <version>1.2.2</version>
</dependency>
```

### Gradle

```groovy
// build.gradle
implementation 'com.qiplat:sweetline-ffm:1.2.2'
```

---

## 环境要求

- JDK 22
- 运行参数：
  - `--enable-preview`
  - `--enable-native-access=ALL-UNNAMED`
- 需要确保 native 库 `sweetline` 可被加载（见 [Native 加载](#native-加载)）

---

## 构建与运行

```bash
cd platform/Java22
./gradlew :sweetline:build
./gradlew :demo:run
```

---

## 核心类型

- `HighlightConfig(boolean showIndex, boolean inlineStyle)`
- `HighlightEngine`
- `Document`（`AutoCloseable`）
- `TextAnalyzer`（`AutoCloseable`）
- `DocumentAnalyzer`（`AutoCloseable`）
- `TextPosition`、`TextRange`、`TextLineInfo`
- `TokenSpan`、`LineHighlight`、`DocumentHighlight`
- `LineRange`、`DocumentHighlightSlice`
- `IndentGuideLine`、`IndentGuideResult`、`LineScopeState`
- `SyntaxCompileError`

说明：
- Java 22 版本的 `HighlightConfig` 当前仅包含 `showIndex` 与 `inlineStyle`。

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

    public TextAnalyzer createAnalyzerBySyntaxName(String syntaxName);
    public TextAnalyzer createAnalyzerByFileName(String fileName);
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
    public DocumentHighlightSlice getHighlightSlice(LineRange visibleRange);
    public IndentGuideResult analyzeIndentGuides();
    public void close();
}
```

`analyzeIncrementalInLineRange(...)` 用于“应用补丁并立即返回切片”。
`getHighlightSlice(...)` 用于在 `analyze()` 或 `analyzeIncremental(...)` 之后，从最近缓存结果中读取可见切片。

---

## Native 加载

`SweetLineNative` 的 native 库加载顺序：

1. 系统属性 `sweetline.lib.path`
2. 常见源码/构建输出目录
3. JAR 资源自动解包回退（`NativeLibraryExtractor.extractToDefaultDir()`）
4. `java.library.path` / `System.loadLibrary`

可显式指定：

```bash
-Dsweetline.lib.path=/path/to/native/lib/dir
```

JAR 打包场景：

```java
import com.qiplat.sweetline.NativeLibraryExtractor;
import java.nio.file.Path;

Path libPath = NativeLibraryExtractor.extractToDefaultDir();
```

---

## 完整示例

```java
import com.qiplat.sweetline.*;

String sourceCode = "public class Demo {}";

try (HighlightEngine engine = new HighlightEngine(new HighlightConfig(true, false))) {
    engine.registerStyleName("keyword", 1);
    engine.registerStyleName("string", 2);
    engine.compileSyntaxFromFile("syntaxes/java.json");

    try (TextAnalyzer textAnalyzer = engine.createAnalyzerBySyntaxName("java")) {
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
                DocumentHighlightSlice cachedVisible = analyzer.getHighlightSlice(
                        new LineRange(0, 80));

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
