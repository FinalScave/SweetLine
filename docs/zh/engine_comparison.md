# SweetLine 与主流语法高亮引擎对比

SweetLine 是一套面向真实产品交付的跨平台语法高亮引擎。它将语法规则编译、复杂状态控制、增量更新、多端封装和产品接口能力整合进同一个核心。

SweetLine 以统一高亮内核提供高性能静态高亮、复杂状态控制、真实增量更新、低运行阶段内存，以及面向 Android、Apple、.NET、Java、Flutter、WASM、OHOS 等平台的产品化交付能力。

## SweetLine 的产品定位

- 它是一套跨平台语法高亮内核，而不是单一宿主环境里的高亮库。
- 它是一套开放的语法规则系统，而不是把规则写死在引擎里的封闭实现。
- 它是一套面向编辑器、查看器、代码浏览组件和 SDK 的产品级高亮方案，而不只是静态代码块渲染工具。

SweetLine 提供丰富的语法规则集，当前已覆盖 `100+` 语言与语法场景。规则由 `HighlightEngine` 编译后使用，开发者也可以自行编写、扩展和加载自己的语法规则。

## 与主流路线的核心差异

### SweetLine vs Prism / highlight.js

`Prism` 和 `highlight.js` 主要面向前端静态高亮场景，适合文档站和网页代码块。

SweetLine 在静态全量高亮上的性能已经非常接近 `Prism`；与此同时，它在复杂语法、状态切换、嵌套语言、专用语法和增量更新方面更适合真实产品场景。

### SweetLine vs TextMate / Shiki

`TextMate / Shiki` 的核心优势是 grammar 与 theme 生态成熟，适合复用大量现成规则与主题。

SweetLine 与 TextMate 同属高表达力 lexical / state machine 路线，但 SweetLine 的状态建模更显式，支持 `subStates`、状态跳转、`onLineEndState`、多行状态传播，以及通过 `importSyntax` 直接导入已编译的语法规则。这使 SweetLine 不只是能描述复杂状态，还能更自然地复用既有 grammar，特别适合嵌套语言、模板语言、组件语法和多语言混排场景。在复杂 stateful grammar、专用语法和多端产品集成上，SweetLine 的可控性更强。

### SweetLine vs Tree-sitter / Lezer

`Tree-sitter / Lezer` 属于 parser 路线，具备很强的结构级语法表达能力。

SweetLine 是一套状态机语法分析引擎，支持显式状态切换、子状态组织、嵌套语法切换和多行状态延续。在语法高亮场景里，SweetLine 具备很强的结构表达能力，同时保持了更高的规则灵活性和更直接的产品落地路径。

## 多维度对比

| 维度 | SweetLine | VS Code TextMate / Shiki | Tree-sitter / Lezer | Prism / highlight.js | Pygments |
| --- | --- | --- | --- | --- | --- |
| 跨平台产品化 | 很强 | 中 | 中到强 | 中 | 弱到中 |
| 语法规则表达力 | 很强 | 强 | 很强 | 中 | 中到高 |
| 复杂状态控制 | 很强 | 强 | 很强 | 弱到中 | 中 |
| 嵌套语言能力 | 很强 | 强 | 强 | 中 | 中 |
| 增量更新能力 | 很强 | 中到强 | 强 | 不支持 | 不支持 |
| 静态全量性能 | 第一梯队 | 中到强 | 中 | 第一梯队 | 中 |
| 阶段内存控制 | 很强 | 弱 | 中 | 中 | 中 |
| 生态成熟度 | 强 | 很强 | 强 | 强 | 强 |
| 产品接口完整度 | 很强 | 中到强 | 强 | 中 | 中到强 |

## 规则系统与精度能力

SweetLine 不是“高亮规则写死在引擎里”的方案，而是一个开放、可编译的语法规则平台。SweetLine 提供丰富的语法规则集，同时允许开发者通过 `HighlightEngine` 编译并加载自定义规则。

它的规则能力并不只是关键词表或简单 token 匹配，而是包含：

- `variables`
- `fragments / include / includes`
- `importSyntax`
- `subStates`
- `onLineEndState`
- capture group 到样式的映射
- 多行状态传播
- 零宽匹配状态跳转
- 文件名 / 后缀 / pattern 路由

SweetLine 的精度能力不属于普通 lexical 高亮范畴，核心特征包括：

- parser 路线在结构精度上限上具备天然优势。
- 在非 parser 路线里，SweetLine 属于最强一档。
- 在复杂状态控制、嵌套切换、专用语法塑形和多行状态传播上，SweetLine 的可控精度上限可以高于传统 TextMate grammar。

这也是 SweetLine 特别适合模板语言、配置语言、嵌套语言、专用 DSL 和编辑器式高亮场景的原因。

## API 与产品接口能力

SweetLine 的接口设计明显面向产品和编辑器场景，而不是只做“输入文本、输出 HTML”。

核心能力包括：

- `HighlightEngine`
- `TextAnalyzer`
- `DocumentAnalyzer`
- `Document`
- `analyzeText`
- `analyze`
- `analyzeIncremental`
- `analyzeLineRange`
- `getHighlightSlice`
- `analyzeIncrementalInLineRange`
- 缩进引导分析
- C API
- 多平台包装 API

SweetLine 提供的是完整高亮内核的接口面：既能做全量分析，也能做文档级增量更新，还能服务切片渲染和多平台绑定。

## 性能表现

以下结果来自统一 benchmark，在共同样本上比较 `Full Cold`、`Full Warm`、`增量更新（Stateful Edit）`、`32x Full Warm` 和 `32x 增量更新（Stateful Edit）`。数据均基于 `Release` 版 SweetLine；`-` 表示该引擎不提供对应的真实增量更新能力。`Stage Memory` 表示从“无引擎”开始到完成一次目标高亮为止的阶段 RSS 增量。

性能概览：

- `Full Cold` 与 `Full Warm` 中，SweetLine 已与最快的静态高亮路线处于同一竞争区间。
- `增量更新（Stateful Edit）` 与 `32x 增量更新（Stateful Edit）` 中，SweetLine 均取得当前最优结果。
- 在统一口径下，SweetLine 也保持了最突出的阶段内存控制能力。

### Full Cold

| 引擎 | Mean | Median | p95 | Stage Memory |
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

| 引擎 | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `0.598 ms` | `0.663 ms` | `1.023 ms` | `0.987 MiB` |
| VS Code TextMate | `4.740 ms` | `5.224 ms` | `8.074 ms` | `42.052 MiB` |
| Shiki | `40.590 ms` | `43.575 ms` | `68.964 ms` | `69.686 MiB` |
| Tree-sitter | `1.261 ms` | `1.091 ms` | `3.136 ms` | `10.538 MiB` |
| CodeMirror / Lezer | `1.441 ms` | `1.257 ms` | `2.791 ms` | `18.488 MiB` |
| highlight.js | `1.376 ms` | `1.171 ms` | `2.537 ms` | `21.896 MiB` |
| Prism | `0.562 ms` | `0.423 ms` | `1.010 ms` | `8.548 MiB` |
| Pygments | `3.114 ms` | `3.270 ms` | `5.318 ms` | `3.184 MiB` |

### 增量更新（Stateful Edit）

| 引擎 | Mean | Median | p95 | Stage Memory |
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

| 引擎 | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `18.715 ms` | `20.979 ms` | `29.903 ms` | `5.192 MiB` |
| VS Code TextMate | `131.568 ms` | `147.833 ms` | `182.182 ms` | `47.949 MiB` |
| Shiki | `1310.519 ms` | `1384.627 ms` | `2247.328 ms` | `105.307 MiB` |
| Tree-sitter | `38.769 ms` | `34.417 ms` | `89.823 ms` | `27.833 MiB` |
| CodeMirror / Lezer | `29.612 ms` | `30.551 ms` | `54.839 ms` | `30.630 MiB` |
| highlight.js | `26.579 ms` | `22.661 ms` | `43.509 ms` | `56.443 MiB` |
| Prism | `8.257 ms` | `7.096 ms` | `14.869 ms` | `27.968 MiB` |
| Pygments | `98.131 ms` | `101.055 ms` | `163.714 ms` | `5.197 MiB` |

### 32x 增量更新（Stateful Edit）

| 引擎 | Mean | Median | p95 | Stage Memory |
| --- | --- | --- | --- | --- |
| SweetLine | `0.549 ms` | `0.450 ms` | `1.137 ms` | `1.482 MiB` |
| VS Code TextMate | `19.394 ms` | `0.121 ms` | `94.509 ms` | `51.909 MiB` |
| Shiki | `-` | `-` | `-` | `-` |
| Tree-sitter | `7.908 ms` | `2.350 ms` | `32.615 ms` | `34.538 MiB` |
| CodeMirror / Lezer | `12.629 ms` | `11.504 ms` | `31.993 ms` | `31.962 MiB` |
| highlight.js | `-` | `-` | `-` | `-` |
| Prism | `-` | `-` | `-` | `-` |
| Pygments | `-` | `-` | `-` | `-` |

## 适用场景

SweetLine 特别适合以下场景：

- 跨平台代码编辑器
- 跨平台代码查看器
- 代码浏览组件与 SDK
- 需要复杂状态语法和专用语法支持的产品
- 需要真增量更新能力的编辑器型应用
- 希望统一 Android、Apple、.NET、Java、Flutter、WASM、OHOS 高亮核心的项目

部分场景更适合其他路线：

- 网页静态代码块渲染：`Prism`、`Shiki`、`highlight.js`
- 追求完整 parser 路线的结构上限：`Tree-sitter / Lezer`

对于真正可交付、可扩展、可多端复用的语法高亮核心，SweetLine 更适合相关产品场景。

## 结论

SweetLine 将跨平台交付、开放且可编译的语法规则系统、复杂状态控制、增量更新能力、低阶段内存和产品级接口整合为统一高亮内核：

- 跨平台交付
- 开放且可编译的语法规则系统
- 复杂状态控制与高表达力 grammar
- 强增量更新能力
- 低阶段内存
- 完整的产品与编辑器接口

它既能胜任静态代码高亮，也能支撑编辑器、代码查看器、代码浏览组件和多端 SDK 等更复杂的产品场景。

凭借高表达力的状态机语法分析能力、灵活的 grammar 复用机制和面向真实运行环境优化的性能表现，SweetLine 适合复杂语法、高频编辑、长文档更新和跨平台复用场景。
