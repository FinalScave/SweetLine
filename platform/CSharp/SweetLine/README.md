# SweetLine for .NET (C# / WinForms)

SweetLine is a high-performance syntax highlighting engine for modern editors.  
This package provides the C# wrapper over the SweetLine native core.

## Features

- Full-text highlighting (`TextAnalyzer`)
- Incremental highlighting (`DocumentAnalyzer`)
- Scope/indent guide analysis
- Style-ID and inline-style output modes

## Install

```bash
dotnet add package SweetLine
```

## Quick Start

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
    DocumentHighlight highlight = analyzer.Analyze();
    IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
}
```

## Incremental Highlighting

```csharp
var changeRange = new TextRange(
    new TextPosition(line: 2, column: 4),
    new TextPosition(line: 2, column: 8));

DocumentHighlight updated = analyzer!.AnalyzeIncremental(changeRange, "modified");
DocumentHighlightSlice visible = analyzer.AnalyzeIncrementalInLineRange(
    changeRange,
    "modified",
    new LineRange(startLine: 0, lineCount: 120));
```

## Native Library

- This package currently bundles native library for: `win-x64`
- Bundled file path inside package: `runtimes/win-x64/native/sweetline.dll`

Native resolver search order in `SweetLineNative`:

1. `SWEETLINE_LIB_PATH` (file or directory)
2. App base / current directory candidates
3. Runtime native folders and common build output folders
4. System default loader

If needed, set explicit path:

```powershell
$env:SWEETLINE_LIB_PATH = "C:\\path\\to\\native"
```

## Requirements

- .NET 8.0+
- Windows x64 (for bundled native binary in current package)

## Maintainer: Pack and Publish

1. Build native library to:
   - `cmake-build-release-visual-studio/bin/sweetline.dll`
2. Pack:

```bash
dotnet pack platform/CSharp/SweetLine/SweetLine.csproj -c Release -o artifacts/nuget /p:PackageVersion=0.1.0
```

3. Push:

```powershell
$env:NUGET_API_KEY = "<your-nuget-api-key>"
dotnet nuget push artifacts/nuget/SweetLine.0.1.0.nupkg --api-key $env:NUGET_API_KEY --source https://api.nuget.org/v3/index.json --skip-duplicate
```

## Links

- Repository: https://github.com/FinalScave/SweetLine
- Full docs: https://github.com/FinalScave/SweetLine/tree/main/docs
