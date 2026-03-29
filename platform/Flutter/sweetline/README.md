# sweetline

SweetLine Flutter/Dart FFI package.

It wraps the SweetLine native C API and provides a Dart-friendly API for:

- full text syntax highlighting
- single-line analysis
- managed document incremental analysis
- visible-range highlight slice retrieval
- indent guide analysis

## Features

- `HighlightEngine` for syntax compilation and analyzer creation
- `TextAnalyzer` for full text and single-line analysis
- `Document` + `DocumentAnalyzer` for incremental updates
- `DocumentHighlight` / `DocumentHighlightSlice` / `IndentGuideResult` models
- optional `showIndex` support
- optional inline-style output

## Supported native binaries

The current package build hook bundles prebuilt native libraries for:

- Android `arm64-v8a`
- Android `x86_64`
- Windows `x86_64`

Web is not supported.

## Prepare native binaries

The package build hook reads native libraries from the package-local `native/`
directory. Before running the demo, tests, or publishing the package, sync the
prebuilt binaries from the repository root:

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter/sweetline
dart tool/sync_native_binaries.dart
```

Use `dart tool/...`, not `dart run tool/...`. Running through `dart run` will
trigger build hooks before the `native/` directory is populated.

## Installation

Add the package dependency:

```yaml
dependencies:
  sweetline:
    path: ../sweetline
```

If you publish the package later, replace the local `path` dependency with the published version.

## Load syntax rules

SweetLine does not hardcode language grammars in the Dart layer. You need to provide syntax JSON yourself, then compile it into the engine.

In Flutter applications, loading syntax JSON from assets is the most direct approach:

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
    const HighlightConfig(showIndex: true, inlineStyle: false),
  );

  final dartSyntax = await rootBundle.loadString('assets/syntaxes/dart.json');
  engine.compileSyntaxFromJson(dartSyntax);
  return engine;
}
```

You can also call `compileSyntaxFromFile(...)` if you already have a real file path on the current platform.

## Quick start

```dart
import 'package:flutter/services.dart' show rootBundle;
import 'package:sweetline/sweetline.dart';

Future<void> main() async {
  final engine = HighlightEngine(
    const HighlightConfig(showIndex: true, inlineStyle: false),
  );

  try {
    final syntaxJson = await rootBundle.loadString('assets/syntaxes/dart.json');
    engine.compileSyntaxFromJson(syntaxJson);

    final analyzer = engine.createAnalyzerByName('dart');
    if (analyzer == null) {
      throw StateError('dart syntax is not available');
    }

    final result = analyzer.analyzeText('class Foo<T> {\\n  final int value;\\n}');
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
    engine.close();
  }
}
```

## Incremental document analysis

Use `DocumentAnalyzer` when text changes over time and you need incremental re-analysis.

```dart
import 'package:sweetline/sweetline.dart';

void analyzeDocument(HighlightEngine engine, String source) {
  final document = Document('file:///lib/main.dart', source);

  try {
    final analyzer = engine.loadDocument(document);
    if (analyzer == null) {
      throw StateError('failed to load document');
    }

    final fullHighlight = analyzer.analyze();

    final slice = analyzer.analyzeIncrementalInLineRange(
      TextRange(
        const TextPosition(1, 0),
        const TextPosition(1, 0),
      ),
      '  print(value);\\n',
      const LineRange(0, 40),
    );

    print(fullHighlight.lines.length);
    print(slice.startLine);
    print(slice.totalLineCount);
  } finally {
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
- `createAnalyzerByName(String syntaxName)`
- `createAnalyzerByExtension(String extension)`
- `loadDocument(Document document)`

### `TextAnalyzer`

- `analyzeText(String text)`
- `analyzeLine(String text, TextLineInfo info)`
- `analyzeIndentGuides(String text)`

### `DocumentAnalyzer`

- `analyze()`
- `analyzeIncremental(TextRange range, String newText)`
- `analyzeIncrementalInLineRange(TextRange range, String newText, LineRange visibleRange)`
- `getHighlightSlice(LineRange visibleRange)`
- `analyzeIndentGuides()`

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

- call `engine.close()` when the engine is no longer needed
- call `document.close()` when the managed document is no longer needed
- `TextAnalyzer` and `DocumentAnalyzer` are lightweight wrappers; closing the owning engine/document is the important part

## Notes

- `showIndex: true` makes `TextPosition.index` available in returned spans
- `inlineStyle: true` returns inline color/font style instead of `styleId`
- `createAnalyzerByExtension(...)` only works after loading syntax rules that declare matching `fileExtensions`
