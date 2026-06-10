package com.qiplat.sweetline

private object AndroidSweetLineLibrary {
    init {
        System.loadLibrary("sweetline")
    }

    fun load() = Unit
}

actual class HighlightEngine actual constructor(config: HighlightConfig) {
    private var nativeHandle: Long = 0

    actual constructor() : this(HighlightConfig())

    init {
        AndroidSweetLineLibrary.load()
        nativeHandle = nativeMakeEngine(config.toNativeBits())
    }

    actual fun registerStyleName(styleName: String, styleId: Int) {
        nativeHandle.ifOpen { nativeRegisterStyle(it, styleName, styleId) }
    }

    actual fun getStyleName(styleId: Int): String? {
        return nativeHandle.ifOpenOrNull { nativeGetStyleName(it, styleId) }
    }

    actual fun defineMacro(macroName: String) {
        nativeHandle.ifOpen { nativeDefineMacro(it, macroName) }
    }

    actual fun undefineMacro(macroName: String) {
        nativeHandle.ifOpen { nativeUndefineMacro(it, macroName) }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromJson(syntaxJson: String) {
        nativeHandle.ifOpen { nativeCompileSyntaxFromJson(it, syntaxJson) }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromFile(path: String) {
        nativeHandle.ifOpen { nativeCompileSyntaxFromFile(it, path) }
    }

    actual fun createAnalyzerBySyntaxName(syntaxName: String): TextAnalyzer? {
        val analyzerHandle = nativeHandle.ifOpenOrNull { nativeCreateAnalyzerBySyntaxName(it, syntaxName) } ?: 0
        return analyzerHandle.takeIf { it != 0L }?.let { TextAnalyzer(it) }
    }

    actual fun createAnalyzerByFileName(fileName: String): TextAnalyzer? {
        val analyzerHandle = nativeHandle.ifOpenOrNull { nativeCreateAnalyzerByFileName(it, fileName) } ?: 0
        return analyzerHandle.takeIf { it != 0L }?.let { TextAnalyzer(it) }
    }

    actual fun loadDocument(document: Document): DocumentAnalyzer? {
        val analyzerHandle = nativeHandle.ifOpenOrNull { nativeLoadDocument(it, document.nativeHandle) } ?: 0
        return analyzerHandle.takeIf { it != 0L }?.let { DocumentAnalyzer(it) }
    }

    actual fun close() {
        val handle = nativeHandle
        if (handle != 0L) {
            nativeFinalizeEngine(handle)
            nativeHandle = 0
        }
    }

    fun removeDocument(uri: String) {
        nativeHandle.ifOpen { nativeRemoveDocument(it, uri) }
    }

    companion object {
        @JvmStatic private external fun nativeMakeEngine(configBits: Int): Long
        @JvmStatic private external fun nativeFinalizeEngine(handle: Long)
        @JvmStatic private external fun nativeRegisterStyle(handle: Long, styleName: String, styleId: Int)
        @JvmStatic private external fun nativeGetStyleName(handle: Long, styleId: Int): String?
        @JvmStatic private external fun nativeDefineMacro(handle: Long, macroName: String)
        @JvmStatic private external fun nativeUndefineMacro(handle: Long, macroName: String)
        @JvmStatic private external fun nativeCompileSyntaxFromJson(handle: Long, json: String): Long
        @JvmStatic private external fun nativeCompileSyntaxFromFile(handle: Long, path: String): Long
        @JvmStatic private external fun nativeCreateAnalyzerBySyntaxName(handle: Long, syntaxName: String): Long
        @JvmStatic private external fun nativeCreateAnalyzerByFileName(handle: Long, fileName: String): Long
        @JvmStatic private external fun nativeLoadDocument(handle: Long, documentHandle: Long): Long
        @JvmStatic private external fun nativeRemoveDocument(handle: Long, uri: String)
    }
}

actual class Document actual constructor(uri: String, content: String) {
    internal var nativeHandle: Long = 0
        private set

    init {
        AndroidSweetLineLibrary.load()
        nativeHandle = nativeMakeDocument(uri, content)
    }

    actual fun close() {
        nativeHandle = 0
    }

    companion object {
        @JvmStatic private external fun nativeMakeDocument(uri: String, content: String): Long
        @JvmStatic private external fun nativeGetUri(handle: Long): String?
        @JvmStatic private external fun nativeTotalChars(handle: Long): Int
        @JvmStatic private external fun nativeGetLineCharCount(handle: Long, line: Int): Int
        @JvmStatic private external fun nativeCharIndexOfLine(handle: Long, line: Int): Int
        @JvmStatic private external fun nativeCharIndexToPosition(handle: Long, index: Int): Long
        @JvmStatic private external fun nativeGetLineCount(handle: Long): Int
        @JvmStatic private external fun nativeGetLine(handle: Long, line: Int): String?
        @JvmStatic private external fun nativeGetText(handle: Long): String?
    }
}

actual class TextAnalyzer internal constructor(private var nativeHandle: Long) {
    actual fun analyzeText(text: String): DocumentHighlight {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readDocumentHighlight(nativeAnalyzeText(it, text)) }
            ?: DocumentHighlight()
    }

    actual fun analyzeLine(text: String, info: TextLineInfo): LineAnalyzeResult {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readLineAnalyzeResult(
                buffer = nativeAnalyzeLine(it, text, intArrayOf(info.line, info.startState, info.startCharOffset)),
                lineNumber = info.line,
            )
        } ?: LineAnalyzeResult()
    }

    actual fun analyzeIndentGuides(text: String): IndentGuideResult {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readIndentGuideResult(nativeAnalyzeIndentGuides(it, text)) }
            ?: IndentGuideResult()
    }

    actual fun analyzeBracketPairs(text: String): BracketPairResult {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readBracketPairResult(nativeAnalyzeBracketPairs(it, text)) }
            ?: BracketPairResult()
    }

    actual fun close() {
        val handle = nativeHandle
        if (handle != 0L) {
            nativeFinalize(handle)
            nativeHandle = 0
        }
    }

    companion object {
        @JvmStatic private external fun nativeFinalize(handle: Long)
        @JvmStatic private external fun nativeAnalyzeText(handle: Long, text: String): IntArray?
        @JvmStatic private external fun nativeAnalyzeLine(handle: Long, text: String, info: IntArray): IntArray?
        @JvmStatic private external fun nativeAnalyzeIndentGuides(handle: Long, text: String): IntArray?
        @JvmStatic private external fun nativeAnalyzeBracketPairs(handle: Long, text: String): IntArray?
    }
}

actual class DocumentAnalyzer internal constructor(private var nativeHandle: Long) {
    actual fun analyze(): DocumentHighlight {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readDocumentHighlight(nativeAnalyze(it)) }
            ?: DocumentHighlight()
    }

    actual fun analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readDocumentHighlightSlice(
                nativeAnalyzeLineRange(it, visibleRange.startLine, visibleRange.lineCount),
            )
        } ?: DocumentHighlightSlice()
    }

    actual fun analyzeIncremental(range: TextRange, newText: String): DocumentHighlight {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readDocumentHighlight(
                nativeAnalyzeChanges(it, packTextPosition(range.start), packTextPosition(range.end), newText),
            )
        } ?: DocumentHighlight()
    }

    actual fun analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange,
    ): DocumentHighlightSlice {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readDocumentHighlightSlice(
                nativeAnalyzeChangesInLineRange(
                    handle = it,
                    startPosition = packTextPosition(range.start),
                    endPosition = packTextPosition(range.end),
                    newText = newText,
                    visibleStartLine = visibleRange.startLine,
                    visibleLineCount = visibleRange.lineCount,
                ),
            )
        } ?: DocumentHighlightSlice()
    }

    actual fun getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readDocumentHighlightSlice(
                nativeGetHighlightSlice(it, visibleRange.startLine, visibleRange.lineCount),
            )
        } ?: DocumentHighlightSlice()
    }

    actual fun analyzeIndentGuides(): IndentGuideResult {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readIndentGuideResult(nativeAnalyzeIndentGuides(it)) }
            ?: IndentGuideResult()
    }

    actual fun analyzeIndentGuidesInLineRange(visibleRange: LineRange): IndentGuideResult {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readIndentGuideResult(
                nativeAnalyzeIndentGuidesInLineRange(it, visibleRange.startLine, visibleRange.lineCount),
            )
        } ?: IndentGuideResult()
    }

    actual fun analyzeBracketPairs(): BracketPairResult {
        return nativeHandle.ifOpenOrNull { NativeBufferParser.readBracketPairResult(nativeAnalyzeBracketPairs(it)) }
            ?: BracketPairResult()
    }

    actual fun analyzeBracketPairsInLineRange(visibleRange: LineRange): BracketPairResult {
        return nativeHandle.ifOpenOrNull {
            NativeBufferParser.readBracketPairResultSlice(
                nativeAnalyzeBracketPairsInLineRange(it, visibleRange.startLine, visibleRange.lineCount),
            )
        } ?: BracketPairResult()
    }

    actual fun close() {
        val handle = nativeHandle
        if (handle != 0L) {
            nativeFinalizeAnalyzer(handle)
            nativeHandle = 0
        }
    }

    companion object {
        @JvmStatic private external fun nativeFinalizeAnalyzer(handle: Long)
        @JvmStatic private external fun nativeAnalyze(handle: Long): IntArray?
        @JvmStatic private external fun nativeAnalyzeLineRange(
            handle: Long,
            visibleStartLine: Int,
            visibleLineCount: Int,
        ): IntArray?

        @JvmStatic private external fun nativeAnalyzeChanges(
            handle: Long,
            startPosition: Long,
            endPosition: Long,
            newText: String,
        ): IntArray?

        @JvmStatic private external fun nativeAnalyzeChanges2(
            handle: Long,
            startIndex: Int,
            endIndex: Int,
            newText: String,
        ): IntArray?

        @JvmStatic private external fun nativeAnalyzeChangesInLineRange(
            handle: Long,
            startPosition: Long,
            endPosition: Long,
            newText: String,
            visibleStartLine: Int,
            visibleLineCount: Int,
        ): IntArray?

        @JvmStatic private external fun nativeGetHighlightSlice(
            handle: Long,
            visibleStartLine: Int,
            visibleLineCount: Int,
        ): IntArray?

        @JvmStatic private external fun nativeAnalyzeIndentGuides(handle: Long): IntArray?
        @JvmStatic private external fun nativeAnalyzeIndentGuidesInLineRange(
            handle: Long,
            visibleStartLine: Int,
            visibleLineCount: Int,
        ): IntArray?
        @JvmStatic private external fun nativeAnalyzeBracketPairs(handle: Long): IntArray?
        @JvmStatic private external fun nativeAnalyzeBracketPairsInLineRange(
            handle: Long,
            visibleStartLine: Int,
            visibleLineCount: Int,
        ): IntArray?
        @JvmStatic private external fun nativeGetDocument(handle: Long): Long
    }
}

class SyntaxRule internal constructor(private val nativeHandle: Long) {
    val name: String
        get() = nativeGetName(nativeHandle).orEmpty()

    val fileNames: Array<String>
        get() = nativeGetFileNames(nativeHandle) ?: emptyArray()

    val fileSuffixes: Array<String>
        get() = nativeGetFileSuffixes(nativeHandle) ?: emptyArray()

    companion object {
        @JvmStatic private external fun nativeGetName(handle: Long): String?
        @JvmStatic private external fun nativeGetFileNames(handle: Long): Array<String>?
        @JvmStatic private external fun nativeGetFileSuffixes(handle: Long): Array<String>?
    }
}

private fun HighlightConfig.toNativeBits(): Int {
    var bits = 0
    if (showIndex) {
        bits = bits or 1
    }
    if (inlineStyle) {
        bits = bits or (1 shl 1)
    }
    return bits
}

private fun packTextPosition(position: TextPosition): Long {
    return (position.line.toLong() shl 32) or (position.column.toLong() and 0xffffffffL)
}

private inline fun Long.ifOpen(block: (Long) -> Unit) {
    if (this != 0L) {
        block(this)
    }
}

private inline fun <T> Long.ifOpenOrNull(block: (Long) -> T): T? {
    return if (this != 0L) block(this) else null
}
