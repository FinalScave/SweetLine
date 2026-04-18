# SweetLine Flutter API 文档

本文档说明 SweetLine 在 Flutter / Dart 平台下的集成方式。

---

## Flutter API

Flutter 包通过 Dart FFI 封装 SweetLine C API，并提供与 Android、Java 22、.NET
平台风格一致的 Dart 对象接口。

### 依赖引入

```yaml
dependencies:
  sweetline: ^1.2.2
```

在 monorepo 本地开发时，也可以使用指向 `platform/Flutter/sweetline` 的
`path` 依赖。

### 导入

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

### Document 与 DocumentAnalyzer

```dart
final document = Document('file:///lib/main.dart', source);
final analyzer = engine.loadDocument(document)!;

final full = analyzer.analyze();
final changed = analyzer.analyzeIncremental(range, newText);
final slice = analyzer.analyzeIncrementalInLineRange(
  range,
  newText,
  const LineRange(0, 100),
);
final visible = analyzer.getHighlightSlice(const LineRange(0, 100));
final guides = analyzer.analyzeIndentGuides();

document.close();
engine.close();
```

### 语法文件加载

Flutter 包本身不内置语法 JSON，需要由应用侧提供，可来自 assets 或真实文件。

典型的 Flutter assets 写法：

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

### 当前支持的平台

当前发布包内置的 native 二进制包括：

- Android `arm64-v8a`
- Android `x86_64`
- Linux `aarch64`
- Linux `x86_64`
- macOS `arm64`
- macOS `x86_64`
- Windows `x86_64`
- iOS 真机 `arm64`
- iOS 模拟器 `arm64`

暂不支持 Web。

### 错误处理

- 语法编译失败时抛出 `SyntaxCompileError`
- 其他 native 侧错误抛出 `SweetLineException`

### 说明

- `engine.close()` 与 `document.close()` 需要显式调用
- `showIndex: true` 时，返回结果中的 `TextPosition.index` 可用
- `inlineStyle: true` 时，返回 inline 颜色/字体属性，而不是仅返回 `styleId`
