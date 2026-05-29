package com.qiplat.sweetline

/**
 * SweetLine highlight engine.
 */
expect class HighlightEngine {
    constructor(config: HighlightConfig = HighlightConfig())
    constructor()

    fun registerStyleName(styleName: String, styleId: Int)
    fun getStyleName(styleId: Int): String?
    fun defineMacro(macroName: String)
    fun undefineMacro(macroName: String)

    @Throws(SyntaxCompileError::class)
    fun compileSyntaxFromJson(syntaxJson: String)

    @Throws(SyntaxCompileError::class)
    fun compileSyntaxFromFile(path: String)

    fun createAnalyzerBySyntaxName(syntaxName: String): TextAnalyzer?
    fun createAnalyzerByFileName(fileName: String): TextAnalyzer?
    fun loadDocument(document: Document): DocumentAnalyzer?
    fun close()
}

/**
 * Managed document with incremental update support.
 */
expect class Document {
    constructor(uri: String, content: String)

    fun close()
}

/**
 * Plain text highlight analyzer.
 */
expect class TextAnalyzer {
    fun analyzeText(text: String): DocumentHighlight
    fun analyzeLine(text: String, info: TextLineInfo): LineAnalyzeResult
    fun analyzeIndentGuides(text: String): IndentGuideResult
    fun close()
}

/**
 * Document highlight analyzer with incremental update support.
 */
expect class DocumentAnalyzer {
    fun analyze(): DocumentHighlight
    fun analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice
    fun analyzeIncremental(range: TextRange, newText: String): DocumentHighlight
    fun analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange,
    ): DocumentHighlightSlice

    fun getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice
    fun analyzeIndentGuides(): IndentGuideResult
    fun analyzeIndentGuidesInLineRange(visibleRange: LineRange): IndentGuideResult
    fun close()
}
