# Changelog

## 1.3.4

- Fix scope-based indent guide columns for tab-indented and visible closing lines.
- Limit visible-range indent guide analysis to the requested lines so off-screen scope endings do not shift visible guides.

## 1.3.3

- Add `TabSize` to `HighlightConfig` and pass it to the native engine.
- Improve iOS xcframework support.

## 1.3.2

- Add `HighlightEngine.RemoveDocument(string)` for removing managed documents and cached analyzers.
- Add deterministic native resource cleanup through `IDisposable`, `Dispose()`, and `Close()` with a finalizer fallback.
