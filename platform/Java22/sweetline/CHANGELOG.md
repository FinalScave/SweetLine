# Changelog

## 1.3.2

- Add `tabSize` to `HighlightConfig` and pass it to the native engine.
- Fix scope-based indent guide columns for tab-indented and visible closing lines.
- Limit visible-range indent guide analysis to the requested lines so off-screen scope endings do not shift visible guides.

## 1.3.1

- Add `HighlightEngine.removeDocument(String)` for removing managed documents and cached analyzers.
- Add deterministic native resource cleanup through `AutoCloseable.close()` with a `finalize()` fallback.
