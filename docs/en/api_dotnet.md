# .NET / WinForms API

This document describes the C# wrapper in `platform/CSharp/SweetLine`.

## Overview

The WinForms/.NET wrapper is a P/Invoke binding over the SweetLine C API:

- Native binding and library resolver: `SweetLineNative.cs`
- Managed API (`HighlightEngine`, `TextAnalyzer`, `DocumentAnalyzer`, `Document`): `SweetLine.cs`
- Data models (`TokenSpan`, `DocumentHighlight`, `IndentGuideResult`, ...): `Models.cs`

## Dependency

### NuGet

```bash
dotnet add package SweetLine --version 1.2.0
```

Or in `.csproj`:

```xml
<ItemGroup>
  <PackageReference Include="SweetLine" Version="1.2.0" />
</ItemGroup>
```

## Core Types

- `HighlightConfig`: engine options (`ShowIndex`, `InlineStyle`)
- `HighlightEngine`: compile syntax, create analyzers, load documents
- `TextAnalyzer`: full-text/single-line analysis
- `DocumentAnalyzer`: full and incremental document analysis
- `Document`: managed document handle used for incremental updates
- `SyntaxCompileError`: thrown when syntax compile fails

## Basic Usage

```csharp
using SweetLine;

var config = new HighlightConfig(showIndex: true, inlineStyle: false);
using var engine = new HighlightEngine(config);

engine.RegisterStyleName("keyword", 1);
engine.RegisterStyleName("string", 2);
engine.CompileSyntaxFromFile("syntaxes/csharp.json");

using var document = new Document("file:///example.cs", "public class Demo {}");
using var analyzer = engine.LoadDocument(document);

if (analyzer != null)
{
    DocumentHighlight result = analyzer.Analyze();
    IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
}
```

## Incremental Analysis

`AnalyzeIncrementalInLineRange(...)` applies a patch and returns the requested slice immediately.
`GetHighlightSlice(...)` reads a visible slice from the latest cached result after `Analyze()` or `AnalyzeIncremental(...)`.

```csharp
var changeRange = new TextRange(
    new TextPosition(line: 2, column: 4),
    new TextPosition(line: 2, column: 8));

DocumentHighlight updated = analyzer!.AnalyzeIncremental(changeRange, "modified");
DocumentHighlightSlice visibleFromCache = analyzer.GetHighlightSlice(
    new LineRange(startLine: 0, lineCount: 120));
DocumentHighlightSlice visible = analyzer.AnalyzeIncrementalInLineRange(
    changeRange,
    "modified",
    new LineRange(startLine: 0, lineCount: 120));
```

## Notes

- Call `Dispose()` / `using` for `HighlightEngine` and `Document`.
- `TextAnalyzer` and `DocumentAnalyzer` handles are managed by the engine; they still implement `IDisposable` for lifecycle consistency.
- Native library lookup:
  - `SWEETLINE_LIB_PATH` (file path or directory)
  - app directory / runtime native directories
  - common project build output directories
