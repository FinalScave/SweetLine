package com.qiplat.sweetline

import kotlinx.cinterop.CPointer
import kotlinx.cinterop.CValue
import kotlinx.cinterop.IntVar
import kotlinx.cinterop.allocArray
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.toKString
import kotlinx.cinterop.useContents
import sweetline.sl_analyzer_handle_t
import sweetline.sl_create_document
import sweetline.sl_create_engine
import sweetline.sl_document_analyze
import sweetline.sl_document_analyze_incremental
import sweetline.sl_document_analyze_incremental_in_line_range
import sweetline.sl_document_analyze_bracket_pairs
import sweetline.sl_document_analyze_bracket_pairs_in_line_range
import sweetline.sl_document_analyze_indent_guides
import sweetline.sl_document_analyze_indent_guides_in_line_range
import sweetline.sl_document_analyze_line_range
import sweetline.sl_document_get_highlight_slice
import sweetline.sl_document_handle_t
import sweetline.sl_engine_compile_file
import sweetline.sl_engine_compile_json
import sweetline.sl_engine_create_text_analyzer
import sweetline.sl_engine_create_text_analyzer_by_file_name
import sweetline.sl_engine_define_macro
import sweetline.sl_engine_get_style_name
import sweetline.sl_engine_handle_t
import sweetline.sl_engine_load_document
import sweetline.sl_engine_remove_document
import sweetline.sl_engine_register_style_name
import sweetline.sl_engine_undefine_macro
import sweetline.sl_free_buffer
import sweetline.sl_free_document
import sweetline.sl_free_document_analyzer
import sweetline.sl_free_engine
import sweetline.sl_free_text_analyzer
import sweetline.sl_syntax_error_t
import sweetline.sl_text_analyze
import sweetline.sl_text_analyze_bracket_pairs
import sweetline.sl_text_analyze_indent_guides
import sweetline.sl_text_analyze_line

actual class HighlightEngine actual constructor(config: HighlightConfig) {
    private var nativeHandle: sl_engine_handle_t = sl_create_engine(config.showIndex, config.inlineStyle)

    actual constructor() : this(HighlightConfig())

    actual fun registerStyleName(styleName: String, styleId: Int) {
        nativeHandle?.let { sl_engine_register_style_name(it, styleName, styleId) }
    }

    actual fun getStyleName(styleId: Int): String? {
        return nativeHandle?.let { sl_engine_get_style_name(it, styleId)?.toKString() }
    }

    actual fun defineMacro(macroName: String) {
        nativeHandle?.let { sl_engine_define_macro(it, macroName) }
    }

    actual fun undefineMacro(macroName: String) {
        nativeHandle?.let { sl_engine_undefine_macro(it, macroName) }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromJson(syntaxJson: String) {
        nativeHandle?.let { checkSyntaxError(sl_engine_compile_json(it, syntaxJson)) }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromFile(path: String) {
        nativeHandle?.let { checkSyntaxError(sl_engine_compile_file(it, path)) }
    }

    actual fun createAnalyzerBySyntaxName(syntaxName: String): TextAnalyzer? {
        val handle = nativeHandle?.let { sl_engine_create_text_analyzer(it, syntaxName) }
        return handle?.let { TextAnalyzer(it) }
    }

    actual fun createAnalyzerByFileName(fileName: String): TextAnalyzer? {
        val handle = nativeHandle?.let { sl_engine_create_text_analyzer_by_file_name(it, fileName) }
        return handle?.let { TextAnalyzer(it) }
    }

    actual fun loadDocument(document: Document): DocumentAnalyzer? {
        val handle = nativeHandle?.let { sl_engine_load_document(it, document.nativeHandle) }
        return handle?.let { DocumentAnalyzer(it) }
    }

    actual fun removeDocument(uri: String) {
        nativeHandle?.let { sl_engine_remove_document(it, uri) }
    }

    actual fun close() {
        nativeHandle?.let { sl_free_engine(it) }
        nativeHandle = null
    }
}

actual class Document actual constructor(uri: String, content: String) {
    internal var nativeHandle: sl_document_handle_t = sl_create_document(uri, content)
        private set

    actual fun close() {
        nativeHandle?.let { sl_free_document(it) }
        nativeHandle = null
    }
}

actual class TextAnalyzer internal constructor(private var nativeHandle: sl_analyzer_handle_t) {
    actual fun analyzeText(text: String): DocumentHighlight {
        val result = nativeHandle?.let { sl_text_analyze(it, text) } ?: return DocumentHighlight()
        return try {
            NativeBufferParser.readDocumentHighlight { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeLine(text: String, info: TextLineInfo): LineAnalyzeResult {
        val result = memScoped {
            val lineInfo = allocArray<IntVar>(3)
            lineInfo[0] = info.line
            lineInfo[1] = info.startState
            lineInfo[2] = info.startCharOffset
            nativeHandle?.let { sl_text_analyze_line(it, text, lineInfo) }
        } ?: return LineAnalyzeResult()
        return try {
            NativeBufferParser.readLineAnalyzeResult(info.line) { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeIndentGuides(text: String): IndentGuideResult {
        val result = nativeHandle?.let { sl_text_analyze_indent_guides(it, text) } ?: return IndentGuideResult()
        return try {
            NativeBufferParser.readIndentGuideResult { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeBracketPairs(text: String): BracketPairResult {
        val result = nativeHandle?.let { sl_text_analyze_bracket_pairs(it, text) } ?: return BracketPairResult()
        return try {
            NativeBufferParser.readBracketPairResult { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun close() {
        nativeHandle?.let { sl_free_text_analyzer(it) }
        nativeHandle = null
    }
}

actual class DocumentAnalyzer internal constructor(private var nativeHandle: sl_analyzer_handle_t) {
    actual fun analyze(): DocumentHighlight {
        val result = nativeHandle?.let { sl_document_analyze(it) } ?: return DocumentHighlight()
        return try {
            NativeBufferParser.readDocumentHighlight { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice {
        val result = memScoped {
            val range = allocArray<IntVar>(2)
            range[0] = visibleRange.startLine
            range[1] = visibleRange.lineCount
            nativeHandle?.let { sl_document_analyze_line_range(it, range) }
        } ?: return DocumentHighlightSlice()
        return try {
            NativeBufferParser.readDocumentHighlightSlice { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeIncremental(range: TextRange, newText: String): DocumentHighlight {
        val result = memScoped {
            val changes = allocArray<IntVar>(4)
            changes[0] = range.start.line
            changes[1] = range.start.column
            changes[2] = range.end.line
            changes[3] = range.end.column
            nativeHandle?.let { sl_document_analyze_incremental(it, changes, newText) }
        } ?: return DocumentHighlight()
        return try {
            NativeBufferParser.readDocumentHighlight { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange,
    ): DocumentHighlightSlice {
        val result = memScoped {
            val changes = allocArray<IntVar>(4)
            changes[0] = range.start.line
            changes[1] = range.start.column
            changes[2] = range.end.line
            changes[3] = range.end.column
            val visible = allocArray<IntVar>(2)
            visible[0] = visibleRange.startLine
            visible[1] = visibleRange.lineCount
            nativeHandle?.let { sl_document_analyze_incremental_in_line_range(it, changes, newText, visible) }
        } ?: return DocumentHighlightSlice()
        return try {
            NativeBufferParser.readDocumentHighlightSlice { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice {
        val result = memScoped {
            val range = allocArray<IntVar>(2)
            range[0] = visibleRange.startLine
            range[1] = visibleRange.lineCount
            nativeHandle?.let { sl_document_get_highlight_slice(it, range) }
        } ?: return DocumentHighlightSlice()
        return try {
            NativeBufferParser.readDocumentHighlightSlice { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeIndentGuides(): IndentGuideResult {
        val result = nativeHandle?.let { sl_document_analyze_indent_guides(it) } ?: return IndentGuideResult()
        return try {
            NativeBufferParser.readIndentGuideResult { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeIndentGuidesInLineRange(visibleRange: LineRange): IndentGuideResult {
        val result = memScoped {
            val range = allocArray<IntVar>(2)
            range[0] = visibleRange.startLine
            range[1] = visibleRange.lineCount
            nativeHandle?.let { sl_document_analyze_indent_guides_in_line_range(it, range) }
        } ?: return IndentGuideResult()
        return try {
            NativeBufferParser.readIndentGuideResult { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeBracketPairs(): BracketPairResult {
        val result = nativeHandle?.let { sl_document_analyze_bracket_pairs(it) } ?: return BracketPairResult()
        return try {
            NativeBufferParser.readBracketPairResult { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun analyzeBracketPairsInLineRange(visibleRange: LineRange): BracketPairResult {
        val result = memScoped {
            val range = allocArray<IntVar>(2)
            range[0] = visibleRange.startLine
            range[1] = visibleRange.lineCount
            nativeHandle?.let { sl_document_analyze_bracket_pairs_in_line_range(it, range) }
        } ?: return BracketPairResult()
        return try {
            NativeBufferParser.readBracketPairResultSlice { index -> result.read(index) }
        } finally {
            sl_free_buffer(result)
        }
    }

    actual fun close() {
        nativeHandle?.let { sl_free_document_analyzer(it) }
        nativeHandle = null
    }
}

private fun checkSyntaxError(error: CValue<sl_syntax_error_t>) {
    error.useContents {
        val code = err_code.toInt()
        if (code != 0) {
            throw SyntaxCompileError(code, err_msg?.toKString().orEmpty())
        }
    }
}

private fun CPointer<IntVar>.read(index: Int): Int {
    return this[index]
}
