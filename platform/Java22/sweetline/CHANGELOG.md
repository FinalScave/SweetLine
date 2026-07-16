# Changelog

## Unreleased

- Add `tabSize` to `HighlightConfig` and pass it to the native engine.

## 1.3.1

- Add `HighlightEngine.removeDocument(String)` for removing managed documents and cached analyzers.
- Add deterministic native resource cleanup through `AutoCloseable.close()` with a `finalize()` fallback.
