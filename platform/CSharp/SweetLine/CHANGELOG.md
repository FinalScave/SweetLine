# Changelog

## 1.3.2

- Add `HighlightEngine.RemoveDocument(string)` for removing managed documents and cached analyzers.
- Add deterministic native resource cleanup through `IDisposable`, `Dispose()`, and `Close()` with a finalizer fallback.
