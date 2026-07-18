# Changelog

## 1.0.0

Initial public release of the SweetLine Kotlin Multiplatform binding.

### Platforms and distribution

- Add a shared Kotlin API for Android, iOS, and JVM desktop applications.
- Support Android API 24 and newer on `arm64-v8a` and `x86_64` through JNI.
- Support iOS 14.0 and newer on arm64 devices and arm64 simulators through Kotlin/Native cinterop.
- Support JVM 22 desktop applications on Windows x64, Linux x64/arm64, and macOS 11.0+ x64/arm64 through the Foreign Function & Memory API.
- Bundle the matching SweetLine native runtime in JVM artifacts and extract it automatically at runtime.
- Add Maven Central publication metadata, source and documentation artifacts, and GPG signing support.
- Add `SweetLineCoreIOS.xcframework` integration for final iOS application linking and embedding.

### Engine configuration and syntax management

- Add `HighlightConfig` options for source indexes, inline styles, and tab size.
- Add syntax rule compilation from JSON strings and files.
- Add `SyntaxCompileError` with structured native error codes for invalid JSON, patterns, states, imports, references, and files.
- Add syntax macro definition and removal.
- Add style name registration and reverse lookup by style ID.
- Add text analyzer creation by syntax name or file name.

### Text and document highlighting

- Add full-text highlighting with per-line token spans.
- Add stateful single-line highlighting with end-state and consumed-character metadata.
- Add managed documents backed by the native SweetLine document model.
- Add complete managed-document analysis.
- Add line-range analysis that prepares and returns a visible highlight slice.
- Add incremental text replacement with complete highlight output.
- Add incremental text replacement that directly returns a visible highlight slice.
- Add highlight slice retrieval from an already analyzed document.
- Add `DocumentHighlight`, `DocumentHighlightSlice`, `LineHighlight`, `TokenSpan`, `TextPosition`, `TextRange`, and `LineRange` result models.
- Support both registered style IDs and inline foreground, background, bold, italic, and strikethrough attributes.

### Editor structure analysis

- Add indent guide analysis for plain text and managed documents.
- Add visible-range indent guide analysis for managed documents.
- Expose guide columns, line ranges, continuation flags, branch points, nesting levels, scope states, scope columns, and indentation levels.
- Add bracket pair analysis for plain text and managed documents.
- Add visible-range bracket pair analysis for managed documents.
- Expose bracket ranges, open/close kinds, nesting depth, matched/unmatched/unknown state, and optional partner ranges for navigation and rainbow bracket rendering.

### Resource lifecycle

- Add explicit, repeat-safe `close()` support for `HighlightEngine`, `Document`, `TextAnalyzer`, and `DocumentAnalyzer`.
- Add `HighlightEngine.removeDocument(String)` on Android, iOS, and JVM targets.
- Release native result buffers after conversion to Kotlin result models.
- Document the required managed-document shutdown order.

### Multiplatform demo

- Add a Compose Multiplatform demo for Android, iOS, and JVM desktop.
- Add generated demo resources containing the SweetLine syntax collection and representative source files.
- Add syntax warmup, file selection, theme selection, and load/analyze timing information.
- Add a custom code canvas with line numbers, highlighted tokens, indent guides, and rainbow bracket rendering.
- Add responsive compact and wide layouts so the demo remains usable on iPhone, Android, and desktop window sizes.
