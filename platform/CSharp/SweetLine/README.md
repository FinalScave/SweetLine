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
- Syntax loading from JSON string or file, plus file-name-based routing

## Install

```bash
dotnet add package SweetLine
```

## Syntax Rules

Built-in syntax rule JSON files are not embedded in the NuGet package.
Download the language rules you need from the repository `syntaxes/` directory:

- [syntaxes/](https://github.com/FinalScave/SweetLine/tree/master/syntaxes)

Then place the selected JSON files in your application resources or deployment directory and load them with `CompileSyntaxFromFile(...)`.
After compiling the syntax rules, prefer `CreateAnalyzerByFileName(...)` or `LoadDocument(...)` with real file names so the core can resolve routing automatically.
If the syntax is already known, `CreateAnalyzerBySyntaxName(...)` can be used to bypass routing.

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

TextAnalyzer? textAnalyzer = engine.CreateAnalyzerByFileName("Program.cs");
if (textAnalyzer is not null)
{
    DocumentHighlight result = textAnalyzer.AnalyzeText("public class Demo {}");
    IndentGuideResult guides = textAnalyzer.AnalyzeIndentGuides("public class Demo {}");
}
```

## Managed Document and Incremental Updates

```csharp
using var document = new Document("Program.cs", sourceText);
DocumentAnalyzer? analyzer = engine.LoadDocument(document);

if (analyzer is not null)
{
    DocumentHighlight initial = analyzer.Analyze();

    var editRange = new TextRange(
        new TextPosition(Line: 2, Column: 4),
        new TextPosition(Line: 2, Column: 8));

    DocumentHighlight updated = analyzer.AnalyzeIncremental(editRange, "newText");

    // Analyze enough lines from the current document state to cover the viewport
    DocumentHighlightSlice analyzedVisible = analyzer.AnalyzeLineRange(
        new LineRange(StartLine: 0, LineCount: 80));

    // Read the current visible window from the latest cached highlight result
    DocumentHighlightSlice visible = analyzer.GetHighlightSlice(
        new LineRange(StartLine: 0, LineCount: 80));

    // Or apply a patch and return only the requested slice in one call
    DocumentHighlightSlice updatedVisible = analyzer.AnalyzeIncrementalInLineRange(
        editRange,
        "newText",
        new LineRange(StartLine: 0, LineCount: 80));

    IndentGuideResult guides = analyzer.AnalyzeIndentGuides();
    IndentGuideResult visibleGuides = analyzer.AnalyzeIndentGuidesInLineRange(
        new LineRange(StartLine: 0, LineCount: 80));
}
```

Use `AnalyzeLineRange(...)` when the editor needs a visible slice and SweetLine should analyze enough lines from the current document state first.
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
| `IndentGuideResult` | `StartLine`, `GuideLines`, and per-line `LineStates` |
| `IndentGuideLine` | `Column`, `StartLine`, `EndLine`, continuation flags, and `Branches` |
| `LineScopeState` | `NestingLevel`, `ScopeState`, `ScopeColumn`, and `IndentLevel` |
| `LineAnalyzeResult` | Single-line highlight, `EndState`, and `CharCount` |

## Native Library

This package bundles:

- `runtimes/win-x64/native/sweetline.dll`
- `runtimes/linux-x64/native/libsweetline.so`
- `runtimes/linux-arm64/native/libsweetline.so`
- `runtimes/osx-x64/native/libsweetline.dylib`
- `runtimes/osx-arm64/native/libsweetline.dylib`
- `runtimes/android-arm64/native/libsweetline.so`
- `runtimes/android-x64/native/libsweetline.so`
- `native/ios/SweetLineCoreIOS.xcframework`

Android native assets are consumed from the NuGet runtime folders by .NET for Android and are placed in the APK/AAB automatically.
iOS native assets are added to consuming application projects through a NuGet `buildTransitive` target:

- `net8.0-ios` consumers receive a `NativeReference` to `SweetLineCoreIOS.xcframework`

Set `SweetLineDisableIosNativeReference=true` in the application project to opt out of the iOS `NativeReference` and provide the native reference yourself.

Desktop native loading checks `SWEETLINE_LIB_PATH`, local application folders, runtime native folders, and the system loader.

Override example:

```powershell
$env:SWEETLINE_LIB_PATH = "C:\path\to\native"
```

## Requirements

- .NET 8.0 or newer
- Windows x64, Linux x64/arm64, macOS x64/arm64, Android arm64/x64, or iOS arm64/simulator-arm64 for bundled native binaries

## Pack and Publish

Build or sync the native libraries under `prebuilt/windows/x64`, `prebuilt/linux/x86_64`,
`prebuilt/linux/aarch64`, `prebuilt/osx/x86_64`, `prebuilt/osx/arm64`,
`prebuilt/android/arm64-v8a`, `prebuilt/android/x86_64`, and `prebuilt/ios` first.
When the iOS prebuilt changes, update `prebuilt/ios/SweetLineCoreIOS.xcframework.zip`;
`dotnet pack` extracts it into the NuGet package automatically.

```bash
dotnet pack platform/CSharp/SweetLine/SweetLine.csproj -c Release -o artifacts/nuget /p:PackageVersion=1.2.7

dotnet nuget push artifacts/nuget/SweetLine.1.2.7.nupkg \
  --api-key $NUGET_API_KEY \
  --source https://api.nuget.org/v3/index.json \
  --skip-duplicate
```

## Links

- Repository: [FinalScave/SweetLine](https://github.com/FinalScave/SweetLine)
- .NET API docs: [docs/en/api_dotnet.md](https://github.com/FinalScave/SweetLine/blob/master/docs/en/api_dotnet.md)
- General docs: [docs](https://github.com/FinalScave/SweetLine/tree/master/docs)
