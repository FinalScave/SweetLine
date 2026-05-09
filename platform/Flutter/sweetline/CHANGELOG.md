## 1.2.4

* Align the package version with the latest cross-platform release.
* Update README installation and routing guidance for `createAnalyzerByFileName(...)` and `createAnalyzerBySyntaxName(...)`.
* Add `DocumentAnalyzer.analyzeLineRange(...)` and document its slice-analysis behavior.

## 1.2.2

* Align the package version with the latest cross-platform release.
* Update README examples to use file-name-based analyzer routing.

## 1.0.2

* Add Linux `aarch64` native library support.

## 1.0.1

* Add Linux native library support in the Flutter build hook.
* Add iOS simulator `arm64` native library selection in the Flutter build hook.
* Improve package documentation for syntax rule sourcing and publishing.

## 1.0.0

* Initial Flutter/Dart FFI wrapper for the SweetLine native engine.
* Add `HighlightEngine` for syntax compilation, macro definition, style registration, and analyzer creation.
* Add `TextAnalyzer` for full text, single-line, and indent guide analysis.
* Add `Document` and `DocumentAnalyzer` for managed document loading, incremental analysis, and visible-range highlight slices.
* Add Dart models for document highlights, token spans, inline styles, line states, and indent guide results.
* Add native buffer parsing and native error / syntax compile error handling.
