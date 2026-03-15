# SweetLine Android API 文档

本文档描述 Android (Java/Kotlin) 平台 API。

---

## Android API

Android 通过 JNI 绑定提供 Java API，类名和函数名与 C++ API 保持一致。

### 依赖引入

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:0.1.2'
```

或通过源码依赖，将 `platform/Android/sweetline` 模块导入项目。

### 包名

```java
import com.qiplat.sweetline.*;
```

### HighlightEngine

```java
public class HighlightEngine {
    public HighlightEngine(HighlightConfig config);

    // 编译语法规则
    public SyntaxRule compileSyntaxFromJson(String json) throws SyntaxCompileError;
    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError;

    // 样式管理
    public void registerStyleName(String styleName, int styleId);
    public String getStyleName(int styleId);

    // 宏定义
    public void defineMacro(String macroName);
    public void undefineMacro(String macroName);

    // 创建分析器
    public TextAnalyzer createAnalyzerByName(String syntaxName);
    public TextAnalyzer createAnalyzerByExtension(String extension);
    public DocumentAnalyzer loadDocument(Document document);
    public void removeDocument(String uri);
}
```

### HighlightConfig

```java
public class HighlightConfig {
    public boolean showIndex;     // 是否包含字符索引
    public boolean inlineStyle;   // 是否使用内联样式
    public int tabSize = 4;       // 缩进划线分析的 Tab 宽度
}
```

### Document

```java
public class Document {
    public Document(String uri, String content);

    public String getUri();
    public String getText();
    public int totalChars();
    public int getLineCount();
    public int getLineCharCount(int line);
    public String getLine(int line);
    public int charIndexOfLine(int line);
    public TextPosition charIndexToPosition(int index);
}
```

### TextAnalyzer

```java
public class TextAnalyzer {
    // 全文分析
    public DocumentHighlight analyzeText(String text);

    // 全文分析，直接返回 Android Spannable
    public Spannable analyzeTextAsSpannable(String text, SpannableStyleFactory factory);

    // 单行分析
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info);

    // 缩进划线分析 (内部会先进行高亮分析)
    public IndentGuideResult analyzeIndentGuides(String text);
}
```

### DocumentAnalyzer

```java
public class DocumentAnalyzer {
    // 全量分析
    public DocumentHighlight analyze();

    // 增量分析 (通过范围)
    public DocumentHighlight analyzeIncremental(TextRange range, String newText);

    // 增量分析 (通过字符索引)
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText);

    // 增量分析并只返回可见行区域切片
    public DocumentHighlightSlice analyzeIncrementalInLineRange(
            TextRange range, String newText, LineRange visibleRange);

    // Spannable 相关
    public Spannable analyzeAsSpannable(SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText,
                                                    SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(int startIndex, int endIndex, String newText,
                                                    SpannableStyleFactory factory);

    // 缩进划线分析
    public IndentGuideResult analyzeIndentGuides();

    // 获取托管文档
    public Document getDocument();
}
```

### 数据结构

```java
// 文本位置
public class TextPosition {
    public int line;
    public int column;
    public int index;
}

// 文本范围
public class TextRange {
    public TextPosition start;
    public TextPosition end;
}

// 行范围
public class LineRange {
    public int startLine;
    public int lineCount;
}

// 高亮块
public class TokenSpan {
    public TextRange range;
    public int styleId;
    public InlineStyle inlineStyle;  // 仅 inlineStyle 模式
}

// 行高亮
public class LineHighlight {
    public List<TokenSpan> spans;
}

// 文档高亮
public class DocumentHighlight {
    public List<LineHighlight> lines;
}

// 可见行区域高亮切片
public class DocumentHighlightSlice {
    public int startLine;
    public int totalLineCount;
    public List<LineHighlight> lines;
}

// 缩进划线分析结果
public class IndentGuideResult {
    public List<IndentGuideLine> guideLines;
    public List<LineScopeState> lineStates;
}

// 单行分析结果
public class LineAnalyzeResult {
    public LineHighlight highlight;
    public int endState;
    public int charCount;
}

// 行元数据
public class TextLineInfo {
    public int line;
    public int startState;
    public int startCharOffset;
}
```

### 完整 Android 示例

```java
import com.qiplat.sweetline.*;

// 创建引擎
HighlightConfig config = new HighlightConfig();
config.showIndex = true;
HighlightEngine engine = new HighlightEngine(config);

// 注册样式
engine.registerStyleName("keyword", 1);
engine.registerStyleName("string", 2);
engine.registerStyleName("comment", 3);

// 从 assets 读取语法规则并编译
String json = readAssetFile("syntaxes/java.json");
try {
    engine.compileSyntaxFromJson(json);
} catch (SyntaxCompileError e) {
    Log.e("SweetLine", "编译失败: " + e.getMessage());
}

// 方式一: 全量分析
TextAnalyzer textAnalyzer = engine.createAnalyzerByName("java");
DocumentHighlight result = textAnalyzer.analyzeText(sourceCode);
for (LineHighlight line : result.lines) {
    for (TokenSpan span : line.spans) {
        Log.d("SweetLine", String.format("(%d:%d)-(%d:%d) style=%d",
            span.range.start.line, span.range.start.column,
            span.range.end.line, span.range.end.column,
            span.styleId));
    }
}

// 方式二: 增量分析
Document doc = new Document("file:///main.java", sourceCode);
DocumentAnalyzer docAnalyzer = engine.loadDocument(doc);
DocumentHighlight highlight = docAnalyzer.analyze();

// 编辑发生时增量更新
TextRange changeRange = new TextRange(
    new TextPosition(2, 4, 0),
    new TextPosition(2, 8, 0)
);
DocumentHighlight newHighlight = docAnalyzer.analyzeIncremental(changeRange, "modified");

// 方式三: 直接转换为 Spannable (可直接用于 TextView)
Spannable spannable = textAnalyzer.analyzeTextAsSpannable(sourceCode, styleId -> {
    switch (styleId) {
        case 1: return new ForegroundColorSpan(Color.BLUE);   // keyword
        case 2: return new ForegroundColorSpan(Color.GREEN);  // string
        case 3: return new ForegroundColorSpan(Color.GRAY);   // comment
        default: return null;
    }
});
textView.setText(spannable);
```

---

