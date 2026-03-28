# SweetLine for .NET

SweetLine is a high-performance syntax highlighting and code structure analysis engine.
This package provides the .NET / C# binding over the SweetLine native core via P/Invoke.

## Features

- Full-text highlighting with `TextAnalyzer.AnalyzeText(...)`
- Single-line highlighting with `TextAnalyzer.AnalyzeLine(...)`
- Managed-document incremental highlighting with `DocumentAnalyzer`
- Visible-range slice update with `DocumentAnalyzer.AnalyzeIncrementalInLineRange(...)`
- Visible-range cached slice read with `DocumentAnalyzer.GetHighlightSlice(...)`
- Scope and indent guide analysis with `AnalyzeIndentGuides(...)`
- Preprocessor macro support with `DefineMacro(...)` / `UndefineMacro(...)`
- Style-ID mode and inline-style mode through `HighlightConfig`
- Syntax loading from JSON string or file, plus file-extension lookup

## Install

```bash
dotnet add package SweetLine
```

## Syntax Rules

Built-in syntax rule JSON files are not embedded in the NuGet package.
Download the language rules you need from the repository `syntaxes/` directory:

- [syntaxes/](https://github.com/FinalScave/SweetLine/tree/master/syntaxes)

Then place the selected JSON files in your application resources or deployment directory and load them with `CompileSyntaxFromFile(...)`.

## Quick Start

```csharp
using SweetLine;

var config = new HighlightConfig(ShowIndex: true, InlineStyle: false);
using var engine = new HighlightEngine(config);

engine.RegisterStyleName("keyword", 1);
engine.RegisterStyleName("string", 2);
engine.RegisterStyleName("comment", 3);

// Syntax JSON files are not included in the NuGet package.
// Download them from https://github.com/FinalScave/SweetLine/tree/main/syntaxes
engine.CompileSyntaxFromFile("syntaxes/csharp.json");
// or: engine.CompileSyntaxFromJson(jsonString);

TextAnalyzer? textAnalyzer = engine.CreateAnalyzerByName("csharp");
if (textAnalyzer is not null)
{
    DocumentHighlight result = textAnalyzer.AnalyzeText("public class Demo {}");
    IndentGuideResult guides = textAnalyzer.AnalyzeIndentGuides("public class Demo {}");
}
```

## Managed Document and Incremental Updates

```csharp
using var document = new Document("file:///example.cs", sourceText);
DocumentAnalyzer? analyzer = engine.LoadDocument(document);

if (analyzer is not null)
{
    DocumentHighlight initial = analyzer.Analyze();

    var editRange = new TextRange(
        new TextPosition(Line: 2, Column: 4),
        new TextPosition(Line: 2, Column: 8));

    DocumentHighlight updated = analyzer.AnalyzeIncremental(editRange, "newText");

    // Read the current visible window from the latest cached highlight result
    DocumentHighlightSlice visible = analyzer.GetHighlightSlice(
        new LineRange(StartLine: 0, LineCount: 80));

    // Or apply a patch and return only the requested slice in one call
    DocumentHighlightSlice updatedVisible = analyzer.AnalyzeIncrementalInLineRange(
        editRange,
        "newText",
        new LineRange(StartLine: 0, LineCount: 80));

    IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
}
```

Use `GetHighlightSlice(...)` after `Analyze()` or `AnalyzeIncremental(...)` when the editor only needs the current viewport.
Use `AnalyzeIncrementalInLineRange(...)` when you want to apply the edit and fetch the visible slice in one step.

## Single-Line Analysis

```csharp
var lineInfo = new TextLineInfo(Line: 0, StartState: 0, StartCharOffset: 0);
LineAnalyzeResult lineResult = textAnalyzer!.AnalyzeLine("int x = 42;", lineInfo);

// lineResult.Highlight   token spans for this line
// lineResult.EndState    pass into the next line's StartState
// lineResult.CharCount   characters consumed
```

## Output Models

| Type | Description |
|------|-------------|
| `DocumentHighlight` | Full document result with one `LineHighlight` per line |
| `DocumentHighlightSlice` | Visible-range result with `StartLine`, `TotalLineCount`, and sliced lines |
| `LineHighlight` | Token span list for a single line |
| `TokenSpan` | Highlight range plus `StyleId` or `InlineStyle` |
| `IndentGuideResult` | `GuideLines` and per-line `LineStates` |
| `LineAnalyzeResult` | Single-line highlight, `EndState`, and `CharCount` |

## Native Library

This package bundles:

- `runtimes/win-x64/native/sweetline.dll`

Native resolver search order:

1. `SWEETLINE_LIB_PATH` environment variable, either a file path or a directory
2. App base directory / current directory
3. `runtimes/{rid}/native/` folders and common build output directories
4. System default loader

Override example:

```powershell
$env:SWEETLINE_LIB_PATH = "C:\path\to\native"
```

## Requirements

- .NET 8.0 or newer
- Windows x64 for the bundled native binary

## Pack and Publish

Build the native library first so `cmake-build-release-visual-studio/bin/sweetline.dll` exists.

```bash
dotnet pack platform/CSharp/SweetLine/SweetLine.csproj -c Release -o artifacts/nuget /p:PackageVersion=1.2.0

dotnet nuget push artifacts/nuget/SweetLine.1.2.0.nupkg \
  --api-key $NUGET_API_KEY \
  --source https://api.nuget.org/v3/index.json \
  --skip-duplicate
```

## Links

- Repository: [FinalScave/SweetLine](https://github.com/FinalScave/SweetLine)
- .NET API docs: [docs/en/api_dotnet.md](https://github.com/FinalScave/SweetLine/blob/master/docs/en/api_dotnet.md)
- General docs: [docs](https://github.com/FinalScave/SweetLine/tree/master/docs)
