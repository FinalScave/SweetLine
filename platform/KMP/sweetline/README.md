# SweetLine KMP

SweetLine KMP is the Kotlin Multiplatform binding for the native SweetLine syntax highlighting engine. It provides one Kotlin API for full-text highlighting, incremental editor updates, visible-range results, indent guides, and bracket pair analysis.

## Supported platforms

| Kotlin target | Runtime requirement | Architectures | Native integration |
| --- | --- | --- | --- |
| Android | Android 7.0 / API 24+ | arm64-v8a, x86_64 | JNI |
| iOS | iOS 14.0+ | arm64 device, arm64 simulator | Kotlin/Native cinterop and `SweetLineCoreIOS.xcframework` |
| JVM Desktop | Java 22+ on Windows | x64 | Foreign Function & Memory API |
| JVM Desktop | Java 22+ on Linux | x64, arm64 | Foreign Function & Memory API |
| JVM Desktop | Java 22+ on macOS 11.0+ | x64, arm64 | Foreign Function & Memory API |

macOS support is provided by the JVM desktop target. This release does not publish a macOS Kotlin/Native target.

## Installation

Add the dependency to the shared source set:

```kotlin
kotlin {
    sourceSets {
        commonMain.dependencies {
            implementation("com.qiplat:sweetline-kmp:1.0.0")
        }
    }
}
```

Android consumers receive the native library through the Android artifact. JVM desktop artifacts contain the supported native runtimes and extract the matching library automatically.

### iOS application setup

The Kotlin dependency contains the iOS bindings, while the final Xcode application must link and embed the native framework:

1. Extract `SweetLineCoreIOS.xcframework.zip` from the matching SweetLine release.
2. Add `SweetLineCoreIOS.xcframework` to the Xcode application target.
3. Select **Embed & Sign** for the framework.
4. Ensure the application links `SweetLineCoreIOS` for both device and simulator builds.

The repository's `platform/KMP/iosApp` project is a working integration reference. Use the XCFramework rather than a raw iOS dylib in an application target.

## Core configuration

Create one engine for a related set of syntax rules and documents:

```kotlin
val engine = HighlightEngine(
    HighlightConfig(
        showIndex = true,
        inlineStyle = false,
        tabSize = 4,
    ),
)

engine.registerStyleName("keyword", 1)
engine.registerStyleName("string", 2)
engine.defineMacro("MOBILE")
engine.compileSyntaxFromJson(syntaxJson)
```

Configuration options:

- `showIndex` includes source indexes in returned text positions.
- `inlineStyle` returns style attributes embedded in syntax rules instead of relying only on registered style IDs.
- `tabSize` controls tab expansion during indentation analysis.

Syntax rules can be compiled with `compileSyntaxFromJson(...)` or `compileSyntaxFromFile(...)`. Use `defineMacro(...)` and `undefineMacro(...)` before compilation when syntax rules contain platform conditions. Style names can be registered with `registerStyleName(...)` and queried with `getStyleName(...)`.

## Plain text analysis

Create an analyzer by syntax name when the syntax is known, or by file name when SweetLine should select it from the registered syntax rules:

```kotlin
val analyzer = engine.createAnalyzerByFileName("Main.kt")
    ?: error("No syntax is registered for Main.kt")

try {
    val highlight = analyzer.analyzeText(sourceText)
    val indentGuides = analyzer.analyzeIndentGuides(sourceText)
    val bracketPairs = analyzer.analyzeBracketPairs(sourceText)
} finally {
    analyzer.close()
}
```

For streaming or editor pipelines, `analyzeLine(...)` accepts `TextLineInfo` and returns `LineAnalyzeResult` with the line highlight, end state, and character count. Pass the previous line's end state into the next line to preserve multi-line syntax state.

## Managed documents and incremental updates

Managed documents keep native parsing state between edits:

```kotlin
val uri = "Main.kt"
val document = Document(uri, sourceText)
val analyzer = engine.loadDocument(document)
    ?: error("No syntax is registered for Main.kt")

try {
    val initial = analyzer.analyze()

    val updated = analyzer.analyzeIncremental(
        range = TextRange(
            start = TextPosition(line = 4, column = 8),
            end = TextPosition(line = 4, column = 13),
        ),
        newText = "updatedValue",
    )

    val viewport = analyzer.getHighlightSlice(
        LineRange(startLine = 40, lineCount = 80),
    )
} finally {
    analyzer.close()
    engine.removeDocument(uri)
    document.close()
    engine.close()
}
```

Use the document APIs according to the editor workflow:

| API | Purpose |
| --- | --- |
| `analyze()` | Analyze and return the complete document. |
| `analyzeLineRange(...)` | Analyze enough state for a visible range and return that slice. |
| `analyzeIncremental(...)` | Apply a replacement and return the complete updated highlight. |
| `analyzeIncrementalInLineRange(...)` | Apply a replacement and return only the requested visible range. |
| `getHighlightSlice(...)` | Read a visible slice after prior complete or incremental analysis. |

`DocumentHighlightSlice` contains `startLine`, `totalLineCount`, and the returned line highlights so consumers can position the slice in a virtualized editor.

## Indent guides

Both analyzer types expose indent guide analysis:

```kotlin
val fullGuides = analyzer.analyzeIndentGuides()
val visibleGuides = analyzer.analyzeIndentGuidesInLineRange(
    LineRange(startLine = 40, lineCount = 80),
)
```

`IndentGuideResult` provides:

- guide columns and start/end lines;
- continuation flags for guides crossing the requested range;
- branch points for scope transitions;
- per-line nesting level, scope state, scope column, and indentation level.

The line-range API is available on `DocumentAnalyzer`; `TextAnalyzer` analyzes the supplied complete text.

## Bracket pairs

Bracket analysis supports rainbow coloring, unmatched bracket diagnostics, and matching bracket navigation:

```kotlin
val fullPairs = analyzer.analyzeBracketPairs()
val visiblePairs = analyzer.analyzeBracketPairsInLineRange(
    LineRange(startLine = 40, lineCount = 80),
)
```

Each `BracketToken` contains:

- its `TextRange`;
- nesting `depth`;
- `Open` or `Close` kind;
- `Matched`, `Unmatched`, or `Unknown` match state;
- an optional `partnerRange` for the matching token.

Use `depth` to select a rainbow bracket color and `partnerRange` to implement cursor-based bracket navigation. The line-range API is available on `DocumentAnalyzer`; `TextAnalyzer` analyzes the supplied complete text.

## Highlight results and styles

Highlight output is organized as documents, lines, and token spans:

- `DocumentHighlight` contains all `LineHighlight` entries.
- `DocumentHighlightSlice` represents a visible line range.
- `TokenSpan` contains a `TextRange` plus a registered `styleId` or an `InlineStyle`.
- `InlineStyle` exposes foreground and background colors together with bold, italic, and strikethrough flags.
- `TextPosition.index` is populated when `HighlightConfig.showIndex` is enabled.

## Public API overview

| Type | Operations |
| --- | --- |
| `HighlightEngine` | `registerStyleName`, `getStyleName`, `defineMacro`, `undefineMacro`, `compileSyntaxFromJson`, `compileSyntaxFromFile`, `createAnalyzerBySyntaxName`, `createAnalyzerByFileName`, `loadDocument`, `removeDocument`, and `close`. |
| `Document` | Create a managed native document and release it with `close`. |
| `TextAnalyzer` | `analyzeText`, `analyzeLine`, `analyzeIndentGuides`, `analyzeBracketPairs`, and `close`. |
| `DocumentAnalyzer` | `analyze`, `analyzeLineRange`, `analyzeIncremental`, `analyzeIncrementalInLineRange`, `getHighlightSlice`, `analyzeIndentGuides`, `analyzeIndentGuidesInLineRange`, `analyzeBracketPairs`, `analyzeBracketPairsInLineRange`, and `close`. |

All result types are immutable Kotlin data classes after conversion from the native result buffers.

## Error handling

Invalid syntax rules throw `SyntaxCompileError`, which includes the native `errorCode` and message:

```kotlin
try {
    engine.compileSyntaxFromJson(syntaxJson)
} catch (error: SyntaxCompileError) {
    println("Syntax error ${error.errorCode}: ${error.message}")
}
```

The companion object exposes constants for missing or invalid JSON properties, invalid patterns and states, invalid JSON or files, missing imports, missing state references, and missing inline-style references.

## Resource management

`HighlightEngine`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer` own native resources. Their `close()` methods are repeat-safe, but they do not implement `AutoCloseable`, so release them explicitly.

For managed documents, use this order:

1. Close the `DocumentAnalyzer`.
2. Call `HighlightEngine.removeDocument(uri)`.
3. Close the `Document`.
4. Close the `HighlightEngine` after all analyzers and documents using it are gone.

SweetLine converts native result buffers into Kotlin data classes and releases those temporary buffers internally.

## JVM native library loading

On JVM desktop, SweetLine first checks the `sweetline.lib.path` system property, then extracts the native library bundled in the artifact, and finally tries the system library loader.

Override the native directory when needed:

```shell
java -Dsweetline.lib.path=/path/to/native-libraries -jar application.jar
```

The directory must contain the platform library named `sweetline.dll`, `libsweetline.so`, or `libsweetline.dylib`.

## Demo applications

The repository contains a Compose Multiplatform demo for Android, iOS, and JVM desktop. It loads the full syntax collection, switches between representative files and themes, and renders highlighted tokens, line numbers, indent guides, and rainbow brackets.

### Android

```shell
cd platform/KMP
./gradlew :demo:assembleDebug
```

### JVM desktop

The desktop demo requires Java 22:

```shell
cd platform/KMP
./gradlew :demo:jvmRun
```

### iOS

Open `platform/KMP/iosApp/iosApp.xcodeproj` in Xcode, select an iOS device or arm64 simulator, configure the development team for a physical device, and run the `iosApp` scheme.

## Publishing

Maven Central credentials and GPG command-line signing settings must be stored in the user-level `~/.gradle/gradle.properties` file:

```properties
mavenCentralUsername=<Central Portal token username>
mavenCentralPassword=<Central Portal token password>
signing.gnupg.executable=gpg
signing.gnupg.keyName=<signing key ID>
signing.gnupg.passphrase=<signing key passphrase>
```

Use the Central Portal user token rather than the account password. Keep this file private and never commit it to the repository.

Build and upload a deployment for manual release in Central Portal:

```shell
cd platform/KMP
./gradlew :sweetline:publishToMavenCentral
```

To upload and automatically release the deployment after validation:

```shell
cd platform/KMP
./gradlew :sweetline:publishAndReleaseToMavenCentral
```
