# sweetline

SweetLine Flutter/Dart FFI package.

It wraps the SweetLine native C API and provides a Dart-friendly API for:

- full text syntax highlighting
- single-line analysis
- managed document incremental analysis
- visible-range highlight slice retrieval
- indent guide analysis
- bracket pair analysis for rainbow bracket rendering and partner lookup

## Features

- `HighlightEngine` for syntax compilation and analyzer creation
- `TextAnalyzer` for full text and single-line analysis
- `Document` + `DocumentAnalyzer` for incremental updates
- `DocumentHighlight` / `DocumentHighlightSlice` / `IndentGuideResult` / `BracketPairResult` models
- bracket token depth and partner range metadata for rainbow bracket rendering
- optional `showIndex` support
- optional inline-style output
- managed-document removal with `HighlightEngine.removeDocument(...)`
- explicit native resource release with `close()` or `dispose()`

## Supported platforms

The published package bundles prebuilt native libraries for:

- Android `arm64-v8a`
- Android `x86_64`
- Linux `aarch64`
- Linux `x86_64`
- macOS `arm64`
- macOS `x86_64`
- Windows `x86_64`
- iOS device `arm64`
- iOS simulator `arm64`

Web is not supported.
The Apple native binaries require iOS 14.0 or macOS 11.0 and newer.

## Installation

```yaml
dependencies:
  sweetline: ^1.3.1
```

## Load syntax rules

SweetLine does not hardcode language grammars in the Dart layer. You need to
provide syntax JSON yourself, then compile it into the engine.

Built-in syntax rule JSON files can be downloaded from the SweetLine open
source repository's [`syntaxes/`](https://github.com/FinalScave/SweetLine/tree/master/syntaxes)
directory. In most cases you only need to copy the language files your app
actually uses into Flutter assets. After compiling those JSON files, prefer
`createAnalyzerByFileName(...)` or `loadDocument(...)` with real file names so
the core can resolve syntax routing automatically. If the syntax is already
known, `createAnalyzerBySyntaxName(...)` is also available.

In Flutter applications, loading syntax JSON from assets is the most direct
approach:

```yaml
flutter:
  assets:
    - assets/syntaxes/dart.json
```

```dart
import 'package:flutter/services.dart' show rootBundle;
import 'package:sweetline/sweetline.dart';

Future<HighlightEngine> createEngine() async {
  final engine = HighlightEngine(
    const HighlightConfig(showIndex: true, inlineStyle: false, tabSize: 4),
  );

  final dartSyntax = await rootBundle.loadString('assets/syntaxes/dart.json');
  engine.compileSyntaxFromJson(dartSyntax);
  return engine;
}
```

You can also call `compileSyntaxFromFile(...)` if you already have a real file
path on the current platform.

## Quick start

```dart
import 'package:flutter/services.dart' show rootBundle;
import 'package:sweetline/sweetline.dart';

Future<void> main() async {
  final engine = HighlightEngine(
    const HighlightConfig(showIndex: true, inlineStyle: false, tabSize: 4),
  );

  try {
    final syntaxJson = await rootBundle.loadString('assets/syntaxes/dart.json');
    engine.compileSyntaxFromJson(syntaxJson);

    final analyzer = engine.createAnalyzerByFileName('main.dart');
    if (analyzer == null) {
      throw StateError('no syntax matched main.dart');
    }

    try {
      final result = analyzer.analyzeText('class Foo<T> {\n  final int value;\n}');
      for (var line = 0; line < result.lines.length; line++) {
        for (final span in result.lines[line].spans) {
          print(
            'line=$line '
            'column=${span.range.start.column} '
            'length=${span.range.end.column - span.range.start.column} '
            'styleId=${span.styleId}',
          );
        }
      }
    } finally {
      analyzer.close();
    }
  } finally {
    engine.close();
  }
}
```

## Incremental document analysis

Use `DocumentAnalyzer` when text changes over time and you need incremental
re-analysis.

```dart
import 'package:sweetline/sweetline.dart';

void analyzeDocument(HighlightEngine engine, String source) {
  const uri = 'main.dart';
  final document = Document(uri, source);
  DocumentAnalyzer? analyzer;

  try {
    analyzer = engine.loadDocument(document);
    if (analyzer == null) {
      throw StateError('failed to load document');
    }

    final fullHighlight = analyzer.analyze();
    final analyzedSlice = analyzer.analyzeLineRange(const LineRange(0, 40));

    final slice = analyzer.analyzeIncrementalInLineRange(
      TextRange(
        const TextPosition(1, 0),
        const TextPosition(1, 0),
      ),
      '  print(value);\n',
      const LineRange(0, 40),
    );

    print(fullHighlight.lines.length);
    print(analyzedSlice.lines.length);
    print(slice.startLine);
    print(slice.totalLineCount);
  } finally {
    analyzer?.close();
    engine.removeDocument(uri);
    document.close();
  }
}
```

## API overview

### `HighlightEngine`

- `compileSyntaxFromJson(String syntaxJson)`
- `compileSyntaxFromFile(String path)`
- `registerStyleName(String styleName, int styleId)`
- `getStyleName(int styleId)`
- `defineMacro(String macroName)`
- `undefineMacro(String macroName)`
- `createAnalyzerByFileName(String fileName)`
- `createAnalyzerBySyntaxName(String syntaxName)`
- `loadDocument(Document document)`
- `removeDocument(String uri)`

### `TextAnalyzer`

- `analyzeText(String text)`
- `analyzeLine(String text, TextLineInfo info)`
- `analyzeIndentGuides(String text)`
- `analyzeBracketPairs(String text)`

### `DocumentAnalyzer`

- `analyze()`
- `analyzeIncremental(TextRange range, String newText)`
- `analyzeLineRange(LineRange visibleRange)`
- `analyzeIncrementalInLineRange(TextRange range, String newText, LineRange visibleRange)`
- `getHighlightSlice(LineRange visibleRange)`
- `analyzeIndentGuides()`
- `analyzeIndentGuidesInLineRange(LineRange visibleRange)`
- `analyzeBracketPairs()`
- `analyzeBracketPairsInLineRange(LineRange visibleRange)`

## Result models

### `DocumentHighlight`

Full highlight result:

```dart
DocumentHighlight.lines -> List<LineHighlight>
LineHighlight.spans -> List<TokenSpan>
```

### `DocumentHighlightSlice`

Highlight result for a specified visible line range:

```dart
DocumentHighlightSlice.startLine
DocumentHighlightSlice.totalLineCount
DocumentHighlightSlice.lines
```

### `IndentGuideResult`

Indent guide analysis result:

```dart
IndentGuideResult.startLine
IndentGuideResult.guideLines
IndentGuideResult.lineStates
IndentGuideLine.column
IndentGuideLine.startLine
IndentGuideLine.endLine
IndentGuideLine.continuesBefore
IndentGuideLine.continuesAfter
IndentGuideLine.branches
LineScopeState.nestingLevel
LineScopeState.scopeState
LineScopeState.scopeColumn
LineScopeState.indentLevel
```

### `BracketPairResult`

Bracket pair analysis result for rainbow bracket rendering and partner lookup:

```dart
BracketPairResult.startLine
BracketPairResult.totalLineCount
BracketPairResult.lines
LineBracketPairs.tokens
BracketToken.range
BracketToken.depth
BracketToken.kind
BracketToken.matchState
BracketToken.partnerRange
```

### `TokenSpan`

Each token span contains:

- `range`
- `styleId` when `inlineStyle == false`
- `inlineStyle` when `inlineStyle == true`

### `InlineStyle`

Inline style contains:

- `foreground`
- `background`
- `fontAttributes`
- `isBold`
- `isItalic`
- `isStrikethrough`

## Error handling

- `compileSyntaxFromJson(...)` and `compileSyntaxFromFile(...)` throw `SyntaxCompileError` when syntax compilation fails
- other native failures throw `SweetLineException`

## Lifecycle

`HighlightEngine`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer` own native resources. Call `close()` when each object is no longer needed; `dispose()` is an alias.

For managed documents, close the analyzer, call `engine.removeDocument(uri)`, close the document, and finally close the engine.

## Notes

- `showIndex: true` makes `TextPosition.index` available in returned spans
- `inlineStyle: true` returns inline color/font style instead of `styleId`
- `analyzeLineRange(...)` analyzes enough lines from the current document state to satisfy the requested visible range
- `createAnalyzerByFileName(...)` only works after loading syntax rules that declare matching `fileNames`, `fileSuffixes`, or `fileNamePatterns`
- `createAnalyzerByFileName(...)` should be given a basename such as `main.dart`, `Dockerfile`, or `build.gradle.kts`
