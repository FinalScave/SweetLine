# SweetLine vs. Mainstream Syntax Highlighting Engines

SweetLine is a cross-platform syntax highlighting engine built for real product delivery. It unifies grammar compilation, complex state control, incremental updates, multi-platform bindings, and product-grade APIs in a single core.

SweetLine delivers one integrated highlighting core with high-performance static highlighting, advanced state control, true incremental updates, low stage memory, and production-ready delivery across Android, Apple, .NET, Java, Flutter, WASM, and OHOS.

## Product Positioning

- SweetLine is a cross-platform syntax highlighting core rather than a single-host highlighting library.
- SweetLine provides an open grammar system instead of a closed implementation with rules hardcoded into the engine.
- SweetLine is designed for editors, viewers, code browsing components, and SDK scenarios rather than only static code-block rendering.

SweetLine ships with a rich grammar pack covering `100+` languages and syntax scenarios. Grammars are compiled by `HighlightEngine`, and developers can author, extend, and load their own rules as needed.

## Core Differences from Mainstream Routes

### SweetLine vs. Prism / highlight.js

`Prism` and `highlight.js` primarily target front-end static highlighting and fit documentation sites and web code blocks well.

SweetLine already performs very close to `Prism` on full static highlighting while remaining better suited to complex grammars, state transitions, embedded languages, domain-specific syntaxes, and incremental update scenarios.

### SweetLine vs. TextMate / Shiki

`TextMate / Shiki` stand out for their mature grammar and theme ecosystem and for broad reuse of existing grammars and themes.

SweetLine and TextMate both belong to the high-expressiveness lexical / state-machine family, but SweetLine models state more explicitly. It supports `subStates`, explicit state transitions, `onLineEndState`, multi-line state propagation, and direct reuse of compiled grammars through `importSyntax`. That makes SweetLine especially well suited to embedded languages, template syntaxes, component syntaxes, and mixed-language documents. For complex stateful grammars, specialized syntaxes, and multi-platform product integration, SweetLine offers stronger control.

### SweetLine vs. Tree-sitter / Lezer

`Tree-sitter / Lezer` follow the parser route and offer very strong structural syntax representation.

SweetLine is a state-machine-based syntax analysis engine with explicit state transitions, sub-state organization, embedded syntax switching, and multi-line state continuation. In syntax-highlighting scenarios, SweetLine provides strong structural expressiveness while keeping grammar authoring more flexible and product integration more direct.

## Multidimensional Comparison

| Dimension | SweetLine | VS Code TextMate / Shiki | Tree-sitter / Lezer | Prism / highlight.js | Pygments |
| --- | --- | --- | --- | --- | --- |
| Cross-platform productization | Very strong | Medium | Medium to strong | Medium | Weak to medium |
| Grammar expressiveness | Very strong | Strong | Very strong | Medium | Medium to high |
| Complex state control | Very strong | Strong | Very strong | Low to medium | Medium |
| Embedded-language handling | Very strong | Strong | Strong | Medium | Medium |
| Incremental update capability | Very strong | Medium to strong | Strong | Unsupported | Unsupported |
| Static full-highlight performance | Top tier | Medium to strong | Medium | Top tier | Medium |
| Stage memory control | Very strong | Weak | Medium | Medium | Medium |
| Ecosystem maturity | Strong | Very strong | Strong | Strong | Strong |
| Product interface completeness | Very strong | Medium to strong | Strong | Medium | Medium to strong |

## Grammar System and Precision

SweetLine is not a system with highlighting rules hardcoded into the engine. It is an open, compilable grammar platform. SweetLine provides a large grammar set and also allows developers to compile and load custom grammars through `HighlightEngine`.

Its grammar model goes far beyond keyword lists or simple token matching. Core capabilities include:

- `variables`
- `fragments / include / includes`
- `importSyntax`
- `subStates`
- `onLineEndState`
- capture-group-to-style mapping
- multi-line state propagation
- zero-width state transitions
- file-name / suffix / pattern routing

SweetLine should not be viewed as a typical lexical highlighter with a lower ceiling than parser-based routes. Its precision profile can be summarized as follows:

- Parser-based routes have a natural advantage in absolute structural precision ceilings.
- Among non-parser routes, SweetLine belongs to the strongest tier.
- In complex state control, embedded syntax switching, specialized syntax shaping, and multi-line state propagation, SweetLine can offer a higher controllable precision ceiling than traditional TextMate grammars.

That is why SweetLine is especially well suited to template languages, configuration languages, embedded syntaxes, domain-specific languages, and editor-style highlighting workflows.

## API and Product Interfaces

SweetLine is clearly designed for products and editor scenarios rather than only for “input text, output HTML”.

Core capabilities include:

- `HighlightEngine`
- `TextAnalyzer`
- `DocumentAnalyzer`
- `Document`
- `analyzeText`
- `analyze`
- `analyzeIncremental`
- `getHighlightSlice`
- `analyzeIncrementalInLineRange`
- indent guide analysis
- C API
- multi-platform wrapper APIs

SweetLine exposes a complete highlighting-core interface surface: full analysis, document-level incremental updates, visible-range slicing, and multi-platform binding scenarios are all part of the same engine model.

## Performance

The following results come from a unified benchmark covering `Full Cold`, `Full Warm`, `Incremental Update (Stateful Edit)`, `32x Full Warm`, and `32x Incremental Update (Stateful Edit)` on the shared sample set. All figures use the `Release` build of SweetLine. `-` indicates that the engine does not provide a true incremental update path for that scenario. `Stage Memory` measures RSS growth from “no engine loaded” to completion of one target highlighting stage.

Performance overview:

- In `Full Cold` and `Full Warm`, SweetLine already sits in the same competitive band as the fastest static-highlighting route.
- In `Incremental Update (Stateful Edit)` and `32x Incremental Update (Stateful Edit)`, SweetLine delivers the best current result.
- Under a unified measurement method, SweetLine also maintains the strongest stage-memory profile.

### Full Cold

| Engine | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `1.372 ms` | `1.292 ms` | `1.894 ms` | `0.845 MiB` |
| VS Code TextMate | `75.043 ms` | `41.550 ms` | `158.168 ms` | `38.631 MiB` |
| Shiki | `135.812 ms` | `146.652 ms` | `209.383 ms` | `56.540 MiB` |
| Tree-sitter | `3.043 ms` | `1.180 ms` | `14.539 ms` | `9.987 MiB` |
| CodeMirror / Lezer | `3.894 ms` | `2.708 ms` | `9.991 ms` | `14.586 MiB` |
| highlight.js | `3.043 ms` | `1.473 ms` | `9.236 ms` | `20.180 MiB` |
| Prism | `1.228 ms` | `0.732 ms` | `4.661 ms` | `7.173 MiB` |
| Pygments | `5.422 ms` | `3.640 ms` | `14.008 ms` | `3.309 MiB` |

### Full Warm

| Engine | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `0.598 ms` | `0.663 ms` | `1.023 ms` | `0.987 MiB` |
| VS Code TextMate | `4.740 ms` | `5.224 ms` | `8.074 ms` | `42.052 MiB` |
| Shiki | `40.590 ms` | `43.575 ms` | `68.964 ms` | `69.686 MiB` |
| Tree-sitter | `1.261 ms` | `1.091 ms` | `3.136 ms` | `10.538 MiB` |
| CodeMirror / Lezer | `1.441 ms` | `1.257 ms` | `2.791 ms` | `18.488 MiB` |
| highlight.js | `1.376 ms` | `1.171 ms` | `2.537 ms` | `21.896 MiB` |
| Prism | `0.562 ms` | `0.423 ms` | `1.010 ms` | `8.548 MiB` |
| Pygments | `3.114 ms` | `3.270 ms` | `5.318 ms` | `3.184 MiB` |

### Incremental Update (Stateful Edit)

| Engine | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `0.014 ms` | `0.012 ms` | `0.022 ms` | `0.784 MiB` |
| VS Code TextMate | `0.903 ms` | `0.021 ms` | `4.607 ms` | `41.130 MiB` |
| Shiki | `-` | `-` | `-` | `-` |
| Tree-sitter | `0.755 ms` | `0.304 ms` | `2.168 ms` | `10.755 MiB` |
| CodeMirror / Lezer | `1.365 ms` | `1.331 ms` | `2.285 ms` | `17.875 MiB` |
| highlight.js | `-` | `-` | `-` | `-` |
| Prism | `-` | `-` | `-` | `-` |
| Pygments | `-` | `-` | `-` | `-` |

### 32x Full Warm

| Engine | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `18.715 ms` | `20.979 ms` | `29.903 ms` | `5.192 MiB` |
| VS Code TextMate | `131.568 ms` | `147.833 ms` | `182.182 ms` | `47.949 MiB` |
| Shiki | `1310.519 ms` | `1384.627 ms` | `2247.328 ms` | `105.307 MiB` |
| Tree-sitter | `38.769 ms` | `34.417 ms` | `89.823 ms` | `27.833 MiB` |
| CodeMirror / Lezer | `29.612 ms` | `30.551 ms` | `54.839 ms` | `30.630 MiB` |
| highlight.js | `26.579 ms` | `22.661 ms` | `43.509 ms` | `56.443 MiB` |
| Prism | `8.257 ms` | `7.096 ms` | `14.869 ms` | `27.968 MiB` |
| Pygments | `98.131 ms` | `101.055 ms` | `163.714 ms` | `5.197 MiB` |

### 32x Incremental Update (Stateful Edit)

| Engine | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `0.549 ms` | `0.450 ms` | `1.137 ms` | `1.482 MiB` |
| VS Code TextMate | `19.394 ms` | `0.121 ms` | `94.509 ms` | `51.909 MiB` |
| Shiki | `-` | `-` | `-` | `-` |
| Tree-sitter | `7.908 ms` | `2.350 ms` | `32.615 ms` | `34.538 MiB` |
| CodeMirror / Lezer | `12.629 ms` | `11.504 ms` | `31.993 ms` | `31.962 MiB` |
| highlight.js | `-` | `-` | `-` | `-` |
| Prism | `-` | `-` | `-` | `-` |
| Pygments | `-` | `-` | `-` | `-` |

## Best-Fit Scenarios

SweetLine is particularly well suited to:

- cross-platform code editors
- cross-platform code viewers
- code browsing components and SDKs
- products that rely on complex stateful grammars and specialized syntaxes
- editor-style applications that need true incremental updates
- projects that want one highlighting core across Android, Apple, .NET, Java, Flutter, WASM, and OHOS

Some scenarios fit other routes better:

- static web code-block rendering: `Prism`, `Shiki`, `highlight.js`
- maximum structural ceiling from a full parser route: `Tree-sitter / Lezer`

For a truly deliverable, extensible, multi-platform syntax-highlighting core, SweetLine is the stronger fit.

## Conclusion

SweetLine combines cross-platform delivery, an open compilable grammar system, complex state control, incremental updates, low stage memory, and product-grade APIs into one highlighting core:

- cross-platform delivery
- open and compilable grammar system
- complex state control with high-expressiveness grammars
- strong incremental update capability
- low stage memory
- complete product and editor interfaces

It serves both static code highlighting and more demanding product scenarios such as editors, code viewers, code browsing components, and multi-platform SDKs.

With its state-machine-based syntax analysis model, flexible grammar reuse, and performance profile optimized for real runtime conditions, SweetLine is a strong fit for complex grammars, high-frequency editing, long-document updates, and cross-platform reuse.
