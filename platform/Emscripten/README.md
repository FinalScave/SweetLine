# SweetLine for WebAssembly

SweetLine for WebAssembly exposes the native engine through Emscripten Embind and provides JavaScript/TypeScript APIs.

## Build Output

- `sweetline.js`
- `sweetline.wasm`
- `sweetline.d.ts`

Prebuilt files are available under `prebuilt/wasm`.

## Quick Start

```javascript
import createSweetLine from './sweetline.js';

const sl = await createSweetLine();
const engine = new sl.HighlightEngine(new sl.HighlightConfig());
let analyzer;

try {
  engine.compileSyntaxFromJson(syntaxJson);
  analyzer = engine.createAnalyzerByFileName('main.js');
  const result = analyzer.analyzeText(sourceCode);
} finally {
  analyzer?.delete();
  engine.delete();
}
```

## Managed Documents

`HighlightEngine.removeDocument(uri)` removes a managed document and its cached analyzer. Embind objects use `delete()` instead of `close()`.

```javascript
const uri = 'file:///main.js';
const document = new sl.Document(uri, sourceCode);
const analyzer = engine.loadDocument(document);

try {
  const initial = analyzer.analyze();
  const visibleRange = new sl.LineRange();
  visibleRange.startLine = 0;
  visibleRange.lineCount = 80;
  const visible = analyzer.getHighlightSlice(visibleRange);
} finally {
  analyzer.delete();
  engine.removeDocument(uri);
  document.delete();
}
```

## Resource Management

JavaScript garbage collection does not own the underlying C++ objects. Call `delete()` on native-backed Embind objects when they are no longer needed.

For managed documents, use this order:

- delete `DocumentAnalyzer`
- call `HighlightEngine.removeDocument(uri)`
- delete `Document`
- delete `HighlightEngine`

## Build

```bash
./scripts/build-shared.sh --platform wasm
```

See [CHANGELOG.md](CHANGELOG.md) for release notes.
