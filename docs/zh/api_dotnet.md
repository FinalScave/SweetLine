# .NET / WinForms API

本文说明 `platform/CSharp/SweetLine` 下的 C# 封装。

## 概览

WinForms/.NET 封装基于 SweetLine C API 的 P/Invoke：

- Native 绑定与库加载解析：`SweetLineNative.cs`
- 托管 API（`HighlightEngine`、`TextAnalyzer`、`DocumentAnalyzer`、`Document`）：`SweetLine.cs`
- 数据模型（`TokenSpan`、`DocumentHighlight`、`IndentGuideResult` 等）：`Models.cs`

## 依赖引入

### NuGet

```bash
dotnet add package SweetLine --version 1.2.4
```

或在 `.csproj` 中添加：

```xml
<ItemGroup>
  <PackageReference Include="SweetLine" Version="1.2.4" />
</ItemGroup>
```

## 核心类型

- `HighlightConfig`：引擎配置（`ShowIndex`、`InlineStyle`）
- `HighlightEngine`：编译语法、创建分析器、加载文档
- `TextAnalyzer`：纯文本全量/单行分析
- `DocumentAnalyzer`：文档全量与增量分析
- `Document`：用于增量场景的托管文档句柄
- `BracketPairResult`：按行分组的括号 token，可用于彩虹括号渲染和匹配对象查找
- `SyntaxCompileError`：语法编译失败异常

## 基础用法

```csharp
using SweetLine;

var config = new HighlightConfig(showIndex: true, inlineStyle: false);
using var engine = new HighlightEngine(config);

engine.RegisterStyleName("keyword", 1);
engine.RegisterStyleName("string", 2);
engine.CompileSyntaxFromFile("syntaxes/csharp.json");

using var textAnalyzer = engine.CreateAnalyzerByFileName("Example.cs");
DocumentHighlight preview = textAnalyzer!.AnalyzeText("public class Demo {}");

using var document = new Document("file:///example.cs", "public class Demo {}");
using var analyzer = engine.LoadDocument(document);

if (analyzer != null)
{
    DocumentHighlight result = analyzer.Analyze();
    IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
    IndentGuideResult visibleGuides = analyzer.AnalyzeIndentGuidesInLineRange(
        new LineRange(startLine: 0, lineCount: 120));
    BracketPairResult brackets = analyzer.AnalyzeBracketPairs();
    BracketPairResult visibleBrackets = analyzer.AnalyzeBracketPairsInLineRange(
        new LineRange(startLine: 0, lineCount: 120));
}
```

## 增量分析

`AnalyzeLineRange(...)` 会基于当前文档状态分析足够的行，以覆盖请求的可见区。
`AnalyzeIncrementalInLineRange(...)` 会应用补丁并立即返回请求的切片。
`GetHighlightSlice(...)` 则在 `Analyze()` 或 `AnalyzeIncremental(...)` 之后，从最新缓存结果中读取可见切片。
`AnalyzeIndentGuides(...)` 和 `AnalyzeIndentGuidesInLineRange(...)` 会直接基于文本分析缩进划线，不需要先执行高亮分析。
`AnalyzeBracketPairs(...)` 和 `AnalyzeBracketPairsInLineRange(...)` 会直接基于文本分析括号 token，不需要先执行高亮分析。

```csharp
var changeRange = new TextRange(
    new TextPosition(line: 2, column: 4),
    new TextPosition(line: 2, column: 8));

DocumentHighlight updated = analyzer!.AnalyzeIncremental(changeRange, "modified");
DocumentHighlightSlice analyzed = analyzer.AnalyzeLineRange(
    new LineRange(startLine: 0, lineCount: 120));
DocumentHighlightSlice visibleFromCache = analyzer.GetHighlightSlice(
    new LineRange(startLine: 0, lineCount: 120));
DocumentHighlightSlice visible = analyzer.AnalyzeIncrementalInLineRange(
    changeRange,
    "modified",
    new LineRange(startLine: 0, lineCount: 120));
```

## 说明

- `HighlightEngine`、`Document` 请使用 `Dispose()` / `using` 管理生命周期。
- `TextAnalyzer`、`DocumentAnalyzer` 的 native 句柄由 `HighlightEngine` 管理；仍提供 `IDisposable` 以保持生命周期语义一致。
- Native 库加载顺序：
  - `SWEETLINE_LIB_PATH`（文件路径或目录）
  - 应用目录 / runtime native 目录
  - 系统默认 native 解析
