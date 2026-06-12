# Changelog

## 1.3.0

- 新增括号匹配分析元数据，支持彩虹括号渲染和匹配对象查找。

## 1.2.6

- 修复内存泄漏问题。

## 1.2.5

- 更新 indentGuides 分析，支持只分析可见区域。

## 1.2.4

- 同步平台包版本到 `1.2.4`，与当前跨平台发布版本保持一致
- 补充 README 中 `HighlightEngine` 的 API 概览与基于文件名路由的说明
- 补充 `DocumentAnalyzer.analyzeLineRange(...)` 的绑定与文档说明

## 1.2.2

- HighlightEngine.createAnalyzerByExtension 更新为 createAnalyzerByFileName，统一使用基于文件名的自动路由
- HighlightEngine.loadDocument 也使用基于文件名的自动路由
- 同步平台包版本到 `1.2.2` （与C++高亮引擎内核版本对齐）

## 1.1.0

- 补充 `DocumentAnalyzer.analyzeIncrementalInLineRange(...)` 的 README 示例
- 补充 `DocumentAnalyzer.getHighlightSlice(...)` 的 README 示例与 API 说明
- 补充 `DocumentHighlightSlice` / `LineRange` 等可见区切片相关文档

## 1.0.0

- 初始开源发布
- 支持全量高亮分析与增量更新
- 支持单行分析与行状态传递
- 支持缩进引导线分析
- 支持内联样式模式
- 支持 JSON 语法规则编译
- 支持样式名称注册与宏定义
