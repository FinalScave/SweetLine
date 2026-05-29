# SweetLine Flutter API

This document describes Flutter/Dart integration for SweetLine.

---

## Flutter API

The Flutter package wraps the SweetLine C API through Dart FFI and exposes a
Dart-friendly object model aligned with the Android, Java 22, and .NET
wrappers.

### Dependency

```yaml
dependencies:
  sweetline: ^1.2.4
```

For local monorepo development you can also use a `path` dependency pointing to
`platform/Flutter/sweetline`.

### Import

```dart
import 'package:sweetline/sweetline.dart';
```

### HighlightConfig

```dart
const HighlightConfig({
  bool showIndex = false,
  bool inlineStyle = false,
  int tabSize = 4,
})
```

### HighlightEngine

```dart
final engine = HighlightEngine(
  const HighlightConfig(showIndex: true, inlineStyle: false),
);

engine.compileSyntaxFromJson(json);
engine.compileSyntaxFromFile(path);
engine.registerStyleName('keyword', 1);
engine.getStyleName(1);
engine.defineMacro('ANDROID');
engine.undefineMacro('ANDROID');
engine.createAnalyzerBySyntaxName('dart');
engine.createAnalyzerByFileName('main.dart');
engine.loadDocument(document);
engine.close();
```

### TextAnalyzer

```dart
final analyzer = engine.createAnalyzerBySyntaxName('dart');
final full = analyzer!.analyzeText(source);
final line = analyzer.analyzeLine(
  'final int value = 1;',
  const TextLineInfo(line: 0, startState: 0, startCharOffset: 0),
);
final guides = analyzer.analyzeIndentGuides(source);
```

### Document and DocumentAnalyzer

```dart
final document = Document('file:///lib/main.dart', source);
final analyzer = engine.loadDocument(document)!;

final full = analyzer.analyze();
final changed = analyzer.analyzeIncremental(range, newText);
final analyzed = analyzer.analyzeLineRange(const LineRange(0, 100));
final slice = analyzer.analyzeIncrementalInLineRange(
  range,
  newText,
  const LineRange(0, 100),
);
final visible = analyzer.getHighlightSlice(const LineRange(0, 100));
final guides = analyzer.analyzeIndentGuides();
final visibleGuides = analyzer.analyzeIndentGuidesInLineRange(const LineRange(0, 100));

document.close();
engine.close();
```

`analyzeLineRange(...)` analyzes enough lines from the current document state to satisfy the requested visible range.
`getHighlightSlice(...)` only reads the latest cached slice and does not trigger a new analysis.
`analyzeIndentGuides(...)` and `analyzeIndentGuidesInLineRange(...)` do not require a prior highlight pass.

### Syntax loading

The Flutter package does not embed syntax JSON files. You need to provide them
from application assets or real files.

Typical Flutter asset loading:

```yaml
flutter:
  assets:
    - assets/syntaxes/dart.json
```

```dart
import 'package:flutter/services.dart' show rootBundle;

final syntaxJson = await rootBundle.loadString('assets/syntaxes/dart.json');
engine.compileSyntaxFromJson(syntaxJson);
```

### Supported native platforms

The published package currently bundles native binaries for:

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

### Error handling

- syntax compile failures throw `SyntaxCompileError`
- other native-side failures throw `SweetLineException`

### Notes

- call `engine.close()` and `document.close()` explicitly
- `showIndex: true` enables `TextPosition.index` in returned spans
- `inlineStyle: true` returns inline colors/font attributes instead of only
  `styleId`
