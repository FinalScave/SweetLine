package com.qiplat.sweetline

/**
 * Highlight configuration.
 */
data class HighlightConfig(
    val showIndex: Boolean = false,
    val inlineStyle: Boolean = false,
    val tabSize: Int = 4,
)

/**
 * Text position descriptor.
 */
data class TextPosition(
    val line: Int,
    val column: Int,
    val index: Int = 0,
)

/**
 * Text range descriptor.
 */
data class TextRange(
    val start: TextPosition,
    val end: TextPosition,
)

/**
 * Text line metadata for single-line analysis.
 */
data class TextLineInfo(
    val line: Int,
    val startState: Int,
    val startCharOffset: Int = 0,
)

/**
 * Line range descriptor.
 */
data class LineRange(
    val startLine: Int,
    val lineCount: Int,
)

/**
 * Inline style definition embedded in syntax rules.
 */
data class InlineStyle(
    val foreground: Int,
    val background: Int,
    val isBold: Boolean,
    val isItalic: Boolean,
    val isStrikethrough: Boolean,
) {
    constructor(foreground: Int, background: Int, fontAttributes: Int) : this(
        foreground = foreground,
        background = background,
        isBold = (fontAttributes and STYLE_BOLD) != 0,
        isItalic = (fontAttributes and STYLE_ITALIC) != 0,
        isStrikethrough = (fontAttributes and STYLE_STRIKE_THROUGH) != 0,
    )

    companion object {
        const val STYLE_BOLD: Int = 1
        const val STYLE_ITALIC: Int = STYLE_BOLD shl 1
        const val STYLE_STRIKE_THROUGH: Int = STYLE_ITALIC shl 1
    }
}

/**
 * Highlight token span.
 */
data class TokenSpan(
    val range: TextRange,
    val styleId: Int = -1,
    val inlineStyle: InlineStyle? = null,
)

/**
 * Highlight token span sequence for a line.
 */
data class LineHighlight(
    val spans: List<TokenSpan> = emptyList(),
)

/**
 * Highlight result for the entire document.
 */
data class DocumentHighlight(
    val lines: List<LineHighlight> = emptyList(),
)

/**
 * Highlight slice for a line range.
 */
data class DocumentHighlightSlice(
    val startLine: Int = 0,
    val totalLineCount: Int = 0,
    val lines: List<LineHighlight> = emptyList(),
)

/**
 * Bracket token kind.
 */
enum class BracketTokenKind(val value: Int) {
    Open(0),
    Close(1);

    companion object {
        fun fromValue(value: Int): BracketTokenKind {
            return values().firstOrNull { it.value == value } ?: Open
        }
    }
}

/**
 * Bracket match state.
 */
enum class BracketMatchState(val value: Int) {
    Matched(0),
    Unmatched(1),
    Unknown(2);

    companion object {
        fun fromValue(value: Int): BracketMatchState {
            return values().firstOrNull { it.value == value } ?: Unknown
        }
    }
}

/**
 * Single bracket token.
 */
data class BracketToken(
    val range: TextRange,
    val depth: Int,
    val kind: BracketTokenKind,
    val matchState: BracketMatchState,
    val partnerRange: TextRange? = null,
)

/**
 * Bracket token sequence for a line.
 */
data class LineBracketPairs(
    val tokens: List<BracketToken> = emptyList(),
)

/**
 * Bracket pair analysis result.
 */
data class BracketPairResult(
    val startLine: Int = 0,
    val totalLineCount: Int = 0,
    val lines: List<LineBracketPairs> = emptyList(),
)

/**
 * Single line syntax highlight analysis result.
 */
data class LineAnalyzeResult(
    val highlight: LineHighlight = LineHighlight(),
    val endState: Int = 0,
    val charCount: Int = 0,
)

/**
 * Single indent guide line.
 */
data class IndentGuideLine(
    val column: Int,
    val startLine: Int,
    val endLine: Int,
    val continuesBefore: Boolean,
    val continuesAfter: Boolean,
    val branches: List<BranchPoint> = emptyList(),
) {
    data class BranchPoint(
        val line: Int,
        val column: Int,
    )
}

/**
 * Line scope state for indent guide analysis.
 */
data class LineScopeState(
    val nestingLevel: Int,
    val scopeState: Int,
    val scopeColumn: Int,
    val indentLevel: Int,
)

/**
 * Indent guide analysis result.
 */
data class IndentGuideResult(
    val startLine: Int = 0,
    val guideLines: List<IndentGuideLine> = emptyList(),
    val lineStates: List<LineScopeState> = emptyList(),
)

/**
 * Exception thrown when syntax rule compilation fails.
 */
class SyntaxCompileError(
    val errorCode: Int,
    message: String,
) : Exception(message) {
    companion object {
        const val ERR_JSON_PROPERTY_MISSED: Int = -1
        const val ERR_JSON_PROPERTY_INVALID: Int = -2
        const val ERR_PATTERN_INVALID: Int = -3
        const val ERR_STATE_INVALID: Int = -4
        const val ERR_JSON_INVALID: Int = -5
        const val ERR_FILE_NOT_EXISTS: Int = -6
        const val ERR_FILE_INVALID: Int = -7
        const val ERR_IMPORT_SYNTAX_NOT_FOUND: Int = -8
        const val ERR_STATE_REFERENCE_NOT_FOUND: Int = -9
        const val ERR_INLINE_STYLE_REFERENCE_NOT_FOUND: Int = -10
    }
}
