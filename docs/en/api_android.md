# SweetLine Android API

This document describes Android (Java/Kotlin) API usage.

---

## Android API

Android provides a Java API through JNI bindings. Class and function names are consistent with the C++ API.

### Dependency

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:0.1.2'
```

Or import via source dependency by adding the `platform/Android/sweetline` module to your project.

### Package

```java
import com.qiplat.sweetline.*;
```

### HighlightEngine

```java
public class HighlightEngine {
    public HighlightEngine(HighlightConfig config);

    // Compile syntax rules
    public SyntaxRule compileSyntaxFromJson(String json) throws SyntaxCompileError;
    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError;

    // Style management
    public void registerStyleName(String styleName, int styleId);
    public String getStyleName(int styleId);

    // Macro definitions
    public void defineMacro(String macroName);
    public void undefineMacro(String macroName);

    // Create analyzers
    public TextAnalyzer createAnalyzerByName(String syntaxName);
    public TextAnalyzer createAnalyzerByExtension(String extension);
    public DocumentAnalyzer loadDocument(Document document);
    public void removeDocument(String uri);
}
```

### HighlightConfig

```java
public class HighlightConfig {
    public boolean showIndex;     // Whether to include character index
    public boolean inlineStyle;   // Whether to use inline styles
    public int tabSize = 4;       // Tab width for indent guide analysis
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
    // Full text analysis
    public DocumentHighlight analyzeText(String text);

    // Full text analysis, returns Android Spannable directly
    public Spannable analyzeTextAsSpannable(String text, SpannableStyleFactory factory);

    // Single line analysis
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info);

    // Indent guide analysis (internally runs highlight analysis first)
    public IndentGuideResult analyzeIndentGuides(String text);
}
```

### DocumentAnalyzer

```java
public class DocumentAnalyzer {
    // Full analysis
    public DocumentHighlight analyze();

    // Incremental analysis (by range)
    public DocumentHighlight analyzeIncremental(TextRange range, String newText);

    // Incremental analysis (by character index)
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText);

    // Incremental analysis and return only visible line-range slice
    public DocumentHighlightSlice analyzeIncrementalInLineRange(
            TextRange range, String newText, LineRange visibleRange);

    // Spannable variants
    public Spannable analyzeAsSpannable(SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText,
                                                    SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(int startIndex, int endIndex, String newText,
                                                    SpannableStyleFactory factory);

    // Indent guide analysis
    public IndentGuideResult analyzeIndentGuides();

    // Get managed document
    public Document getDocument();
}
```

### Data Structures

```java
// Text position
public class TextPosition {
    public int line;
    public int column;
    public int index;
}

// Text range
public class TextRange {
    public TextPosition start;
    public TextPosition end;
}

// Line range
public class LineRange {
    public int startLine;
    public int lineCount;
}

// Highlight span
public class TokenSpan {
    public TextRange range;
    public int styleId;
    public InlineStyle inlineStyle;  // inlineStyle mode only
}

// Line highlight
public class LineHighlight {
    public List<TokenSpan> spans;
}

// Document highlight
public class DocumentHighlight {
    public List<LineHighlight> lines;
}

// Document highlight slice for visible line range
public class DocumentHighlightSlice {
    public int startLine;
    public int totalLineCount;
    public List<LineHighlight> lines;
}

// Indent guide analysis result
public class IndentGuideResult {
    public List<IndentGuideLine> guideLines;
    public List<LineScopeState> lineStates;
}

// Single line analysis result
public class LineAnalyzeResult {
    public LineHighlight highlight;
    public int endState;
    public int charCount;
}

// Line metadata
public class TextLineInfo {
    public int line;
    public int startState;
    public int startCharOffset;
}
```

### Complete Android Example

```java
import com.qiplat.sweetline.*;

// Create engine
HighlightConfig config = new HighlightConfig();
config.showIndex = true;
HighlightEngine engine = new HighlightEngine(config);

// Register styles
engine.registerStyleName("keyword", 1);
engine.registerStyleName("string", 2);
engine.registerStyleName("comment", 3);

// Read syntax rules from assets and compile
String json = readAssetFile("syntaxes/java.json");
try {
    engine.compileSyntaxFromJson(json);
} catch (SyntaxCompileError e) {
    Log.e("SweetLine", "Compilation failed: " + e.getMessage());
}

// Option 1: Full analysis
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

// Option 2: Incremental analysis
Document doc = new Document("file:///main.java", sourceCode);
DocumentAnalyzer docAnalyzer = engine.loadDocument(doc);
DocumentHighlight highlight = docAnalyzer.analyze();

// Incremental update on edit
TextRange changeRange = new TextRange(
    new TextPosition(2, 4, 0),
    new TextPosition(2, 8, 0)
);
DocumentHighlight newHighlight = docAnalyzer.analyzeIncremental(changeRange, "modified");

// Option 3: Convert to Spannable directly (for use with TextView)
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

