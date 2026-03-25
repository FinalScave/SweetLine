# SweetLine for .NET

SweetLine is a high-performance syntax highlighting and code structure analysis engine.
This package provides the C# binding over the SweetLine native core via P/Invoke.

## Features

- Full-text highlighting (`TextAnalyzer.AnalyzeText`)
- Single-line highlighting (`TextAnalyzer.AnalyzeLine`)
- Incremental highlighting with managed document (`DocumentAnalyzer`)
- Visible-range incremental slice (`DocumentAnalyzer.AnalyzeIncrementalInLineRange`)
- Scope and indent guide analysis (`AnalyzeIndentGuides`)
- Preprocessor macro support (`DefineMacro` / `UndefineMacro`)
- Style-ID and inline-style output modes (`HighlightConfig`)
- Syntax loading from JSON string or file, with file-extension lookup

## Install

```bash
dotnet add package SweetLine
```

## Quick Start

```csharp
using SweetLine;

// 1. Create engine (optionally enable character index or inline style)
var config = new HighlightConfig(ShowIndex: true, InlineStyle: false);
using var engine = new HighlightEngine(config);

// 2. Register style name -> ID mapping
engine.RegisterStyleName("keyword", 1);
engine.RegisterStyleName("string",  2);
engine.RegisterStyleName("comment", 3);

// 3. Compile syntax rules
engine.CompileSyntaxFromFile("syntaxes/csharp.json");
// or: engine.CompileSyntaxFromJson(jsonString);

// 4. Full-text analysis (stateless)
TextAnalyzer? textAnalyzer = engine.CreateAnalyzerByName("csharp");
// or: engine.CreateAnalyzerByExtension(".cs");
if (textAnalyzer != null)
{
    DocumentHighlight result = textAnalyzer.AnalyzeText("public class Demo {}");
    IndentGuideResult guides = textAnalyzer.AnalyzeIndentGuides("public class Demo {}");
}
```

## Managed Document (Incremental)

```csharp
// Create a managed document for incremental updates
using var document = new Document("file:///example.cs", sourceText);
DocumentAnalyzer? analyzer = engine.LoadDocument(document);

// Full analysis
DocumentHighlight highlight = analyzer!.Analyze();

// After an edit, run incremental analysis
var editRange = new TextRange(
    new TextPosition(Line: 2, Column: 4),
    new TextPosition(Line: 2, Column: 8));

DocumentHighlight updated = analyzer.AnalyzeIncremental(editRange, "newText");

// Or get only the visible slice (better for large files)
DocumentHighlightSlice slice = analyzer.AnalyzeIncrementalInLineRange(
    editRange, "newText",
    new LineRange(StartLine: 0, LineCount: 80));

// Indent guide analysis on managed document
IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
```

## Single-Line Analysis

```csharp
// Useful for editor viewports that process one line at a time
var lineInfo = new TextLineInfo(Line: 0, StartState: 0, StartCharOffset: 0);
LineAnalyzeResult lineResult = textAnalyzer!.AnalyzeLine("int x = 42;", lineInfo);

// lineResult.Highlight  — token spans for this line
// lineResult.EndState   — feed into next line's StartState
// lineResult.CharCount  — characters consumed
```

## Preprocessor Macros

```csharp
// Define/undefine macros for #ifdef conditional compilation in syntax rules
engine.DefineMacro("FEATURE_X");
engine.UndefineMacro("FEATURE_X");
```

## Output Models

| Type | Description |
|------|-------------|
| `DocumentHighlight` | Full document: list of `LineHighlight` per line |
| `DocumentHighlightSlice` | Partial result with `StartLine`, `TotalLineCount`, and line slice |
| `LineHighlight` | List of `TokenSpan` for one line |
| `TokenSpan` | Range + `StyleId` (or `InlineStyle` when enabled) |
| `IndentGuideResult` | `GuideLines` (vertical segments with branches) + `LineStates` |
| `LineAnalyzeResult` | Single-line highlight + `EndState` + `CharCount` |

## Native Library

Bundled: `runtimes/win-x64/native/sweetline.dll`

Native resolver search order:

1. `SWEETLINE_LIB_PATH` environment variable (file or directory)
2. App base directory / current directory
3. `runtimes/{rid}/native/` folders and common build output paths
4. System default loader

Override if needed:

```powershell
$env:SWEETLINE_LIB_PATH = "C:\path\to\native"
```

## Requirements

- .NET 8.0+
- Windows x64 (bundled native binary)

## Pack and Publish

```bash
# Build native library first (cmake-build-release-visual-studio/bin/sweetline.dll)

# Pack
dotnet pack platform/CSharp/SweetLine/SweetLine.csproj -c Release -o artifacts/nuget /p:PackageVersion=1.1.1

# Push
dotnet nuget push artifacts/nuget/SweetLine.1.1.1.nupkg \
  --api-key $NUGET_API_KEY \
  --source https://api.nuget.org/v3/index.json \
  --skip-duplicate
```

## Links

- Repository: https://github.com/FinalScave/SweetLine
- Docs: https://github.com/FinalScave/SweetLine/tree/main/docs
