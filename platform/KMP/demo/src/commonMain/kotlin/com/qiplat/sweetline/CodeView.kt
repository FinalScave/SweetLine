package com.qiplat.sweetline.demo

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.ExperimentalTextApi
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.drawText
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.rememberTextMeasurer
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.qiplat.sweetline.BracketMatchState
import com.qiplat.sweetline.BracketPairResult
import com.qiplat.sweetline.BracketToken
import com.qiplat.sweetline.DocumentHighlight
import com.qiplat.sweetline.IndentGuideResult
import com.qiplat.sweetline.TokenSpan
import kotlin.math.ceil
import kotlin.math.max
import kotlin.math.min

@OptIn(ExperimentalTextApi::class)
@Composable
fun CodeView(
    theme: HighlightTheme,
    sourceCode: String,
    highlight: DocumentHighlight?,
    indentGuides: IndentGuideResult?,
    bracketPairs: BracketPairResult?,
    placeholder: String,
    modifier: Modifier = Modifier,
) {
    if (sourceCode.isEmpty() || highlight == null) {
        Box(
            modifier = modifier
                .fillMaxSize()
                .background(theme.background),
            contentAlignment = Alignment.Center,
        ) {
            Text(
                text = placeholder,
                color = theme.lineNumber,
                fontSize = 15.sp,
            )
        }
        return
    }

    val textMeasurer = rememberTextMeasurer()
    val lines = remember(sourceCode) {
        sourceCode.replace("\r\n", "\n").split('\n')
    }
    val textStyle = remember {
        TextStyle(
            fontFamily = FontFamily.Monospace,
            fontSize = 14.sp,
            lineHeight = 20.sp,
        )
    }
    val lineNumberStyle = remember(textStyle) {
        textStyle.copy(fontSize = 12.sp)
    }
    val metrics = remember(sourceCode, textMeasurer, textStyle, lineNumberStyle) {
        CodeViewMetrics.measure(lines, textMeasurer, textStyle, lineNumberStyle)
    }
    val horizontalScroll = rememberScrollState()
    val verticalScroll = rememberScrollState()
    val density = LocalDensity.current
    val canvasWidth = with(density) { metrics.width.toDp() }
    val canvasHeight = with(density) { metrics.height.toDp() }

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(theme.background)
            .verticalScroll(verticalScroll)
            .horizontalScroll(horizontalScroll),
    ) {
        Canvas(modifier = Modifier.size(canvasWidth, canvasHeight)) {
            drawRect(theme.background, size = size)
            drawRect(
                color = theme.gutterBackground,
                topLeft = Offset.Zero,
                size = Size(metrics.gutterWidth, size.height),
            )
            drawLine(
                color = theme.separator,
                start = Offset(metrics.gutterWidth, 0f),
                end = Offset(metrics.gutterWidth, size.height),
                strokeWidth = 1f,
            )

            val codeOriginX = metrics.gutterWidth + metrics.codeLeftPadding
            val codeOriginY = metrics.verticalPadding
            val guideStroke = Stroke(
                width = 1f,
                pathEffect = PathEffect.dashPathEffect(floatArrayOf(3f, 3f)),
            )

            indentGuides?.guideLines.orEmpty().forEach { guide ->
                val innerStart = guide.startLine + 1
                val innerEnd = guide.endLine - 1
                if (innerStart <= innerEnd) {
                    val x = codeOriginX + guide.column * metrics.charWidth
                    val y1 = codeOriginY + innerStart * metrics.lineHeight
                    val y2 = codeOriginY + (min(innerEnd, lines.lastIndex) + 1) * metrics.lineHeight
                    drawLine(
                        color = theme.indentGuide,
                        start = Offset(x, y1),
                        end = Offset(x, y2),
                        strokeWidth = 1f,
                        pathEffect = guideStroke.pathEffect,
                    )
                }
            }

            lines.forEachIndexed { lineIndex, lineText ->
                val y = codeOriginY + lineIndex * metrics.lineHeight
                val bracketTokens = bracketPairs
                    ?.let { pairs -> pairs.lines.getOrNull(lineIndex - pairs.startLine)?.tokens }
                    .orEmpty()
                val lineNumber = (lineIndex + 1).toString()
                val lineNumberWidth = textMeasurer.measure(
                    text = AnnotatedString(lineNumber),
                    style = lineNumberStyle,
                ).size.width.toFloat()
                drawText(
                    textMeasurer = textMeasurer,
                    text = lineNumber,
                    topLeft = Offset(metrics.gutterWidth - 14f - lineNumberWidth, y),
                    style = lineNumberStyle.copy(color = theme.lineNumber),
                )

                drawHighlightedLine(
                    textMeasurer = textMeasurer,
                    lineText = lineText,
                    lineIndex = lineIndex,
                    spans = highlight.lines.getOrNull(lineIndex)?.spans.orEmpty(),
                    theme = theme,
                    textStyle = textStyle,
                    bracketTokens = bracketTokens,
                    x = codeOriginX,
                    y = y,
                )
            }
        }
    }
}

@OptIn(ExperimentalTextApi::class)
private fun androidx.compose.ui.graphics.drawscope.DrawScope.drawHighlightedLine(
    textMeasurer: androidx.compose.ui.text.TextMeasurer,
    lineText: String,
    lineIndex: Int,
    spans: List<TokenSpan>,
    theme: HighlightTheme,
    textStyle: TextStyle,
    bracketTokens: List<BracketToken>,
    x: Float,
    y: Float,
) {
    var currentX = x
    var lastColumn = 0
    spans.forEach { span ->
        val startColumn = span.range.start.column.coerceAtLeast(0)
        val endColumn = if (span.range.start.line == span.range.end.line) {
            min(span.range.end.column, lineText.length)
        } else {
            lineText.length
        }

        if (startColumn > lastColumn && lastColumn < lineText.length) {
            currentX = drawTextSegmentWithBrackets(
                textMeasurer = textMeasurer,
                lineText = lineText,
                startColumn = lastColumn,
                endColumn = min(startColumn, lineText.length),
                x = currentX,
                y = y,
                style = textStyle.copy(color = theme.text),
                bracketTokens = bracketTokens,
            )
        }

        if (span.range.start.line == lineIndex && startColumn < lineText.length && endColumn > startColumn) {
            currentX = drawTextSegmentWithBrackets(
                textMeasurer = textMeasurer,
                lineText = lineText,
                startColumn = startColumn,
                endColumn = endColumn,
                x = currentX,
                y = y,
                style = spanTextStyle(span, theme, textStyle),
                bracketTokens = bracketTokens,
            )
        }

        lastColumn = endColumn
    }

    if (lastColumn < lineText.length) {
        drawTextSegmentWithBrackets(
            textMeasurer = textMeasurer,
            lineText = lineText,
            startColumn = lastColumn,
            endColumn = lineText.length,
            x = currentX,
            y = y,
            style = textStyle.copy(color = theme.text),
            bracketTokens = bracketTokens,
        )
    }
}

@OptIn(ExperimentalTextApi::class)
private fun androidx.compose.ui.graphics.drawscope.DrawScope.drawTextSegment(
    textMeasurer: androidx.compose.ui.text.TextMeasurer,
    text: String,
    x: Float,
    y: Float,
    style: TextStyle,
): Float {
    if (text.isEmpty()) {
        return x
    }
    val result = textMeasurer.measure(text = AnnotatedString(text), style = style)
    drawText(
        textMeasurer = textMeasurer,
        text = text,
        topLeft = Offset(x, y),
        style = style,
    )
    return x + result.size.width
}

private fun spanTextStyle(span: TokenSpan, theme: HighlightTheme, baseStyle: TextStyle): TextStyle {
    val inlineStyle = span.inlineStyle
    if (inlineStyle != null) {
        return baseStyle.copy(
            color = if (inlineStyle.foreground == 0) theme.text else inlineStyle.foreground.toColor(),
            fontWeight = if (inlineStyle.isBold) FontWeight.Bold else FontWeight.Normal,
            fontStyle = if (inlineStyle.isItalic) FontStyle.Italic else FontStyle.Normal,
            textDecoration = if (inlineStyle.isStrikethrough) TextDecoration.LineThrough else TextDecoration.None,
        )
    }
    return baseStyle.copy(color = theme.getColor(span.styleId).toColor())
}

@OptIn(ExperimentalTextApi::class)
private fun androidx.compose.ui.graphics.drawscope.DrawScope.drawTextSegmentWithBrackets(
    textMeasurer: androidx.compose.ui.text.TextMeasurer,
    lineText: String,
    startColumn: Int,
    endColumn: Int,
    x: Float,
    y: Float,
    style: TextStyle,
    bracketTokens: List<BracketToken>,
): Float {
    val start = startColumn.coerceIn(0, lineText.length)
    val end = endColumn.coerceIn(start, lineText.length)
    if (end <= start) {
        return x
    }
    if (bracketTokens.isEmpty()) {
        return drawTextSegment(textMeasurer, lineText.substring(start, end), x, y, style)
    }

    var currentX = x
    var cursor = start
    bracketTokens.forEach { token ->
        val tokenStart = token.range.start.column.coerceIn(0, lineText.length)
        val tokenEnd = token.range.end.column.coerceIn(tokenStart, lineText.length)
        if (tokenEnd <= cursor || tokenStart >= end) {
            return@forEach
        }

        val clippedStart = max(cursor, max(start, tokenStart))
        val clippedEnd = min(end, tokenEnd)
        if (clippedStart > cursor) {
            currentX = drawTextSegment(textMeasurer, lineText.substring(cursor, clippedStart), currentX, y, style)
        }
        if (clippedEnd > clippedStart) {
            currentX = drawTextSegment(
                textMeasurer = textMeasurer,
                text = lineText.substring(clippedStart, clippedEnd),
                x = currentX,
                y = y,
                style = style.copy(color = bracketColor(token)),
            )
        }
        cursor = max(cursor, clippedEnd)
    }

    if (cursor < end) {
        currentX = drawTextSegment(textMeasurer, lineText.substring(cursor, end), currentX, y, style)
    }
    return currentX
}

private val BracketPalette = listOf(
    0xFF7DD3FCu.toInt().toColor(),
    0xFFF9A8D4u.toInt().toColor(),
    0xFFFDE047u.toInt().toColor(),
    0xFF86EFACu.toInt().toColor(),
    0xFFC4B5FDu.toInt().toColor(),
    0xFFFDBA74u.toInt().toColor(),
)

private fun bracketColor(token: BracketToken): Color {
    if (token.matchState == BracketMatchState.Unmatched) {
        return 0xFFFF6B6Bu.toInt().toColor()
    }
    val colorIndex = ((token.depth % BracketPalette.size) + BracketPalette.size) % BracketPalette.size
    val color = BracketPalette[colorIndex]
    return if (token.matchState == BracketMatchState.Unknown) {
        color.copy(alpha = 0.68f)
    } else {
        color
    }
}

@OptIn(ExperimentalTextApi::class)
private data class CodeViewMetrics(
    val lineHeight: Float,
    val charWidth: Float,
    val gutterWidth: Float,
    val codeLeftPadding: Float,
    val verticalPadding: Float,
    val width: Float,
    val height: Float,
) {
    companion object {
        private const val LINE_NUMBER_PADDING = 14f
        private const val CODE_LEFT_PADDING = 12f
        private const val VERTICAL_PADDING = 14f

        fun measure(
            lines: List<String>,
            textMeasurer: androidx.compose.ui.text.TextMeasurer,
            textStyle: TextStyle,
            lineNumberStyle: TextStyle,
        ): CodeViewMetrics {
            val sample = textMeasurer.measure(text = AnnotatedString("0"), style = textStyle)
            val charWidth = max(sample.size.width.toFloat(), 1f)
            val lineHeight = max(sample.size.height.toFloat(), 1f)
            val lineNumberDigits = lines.size.toString().length
            val gutterWidth = lineNumberDigits * charWidth + LINE_NUMBER_PADDING * 2f
            val maxLineWidth = lines.maxOfOrNull { line ->
                val value = if (line.isEmpty()) " " else line
                textMeasurer.measure(text = AnnotatedString(value), style = textStyle).size.width.toFloat()
            } ?: 0f
            val lineNumberWidth = textMeasurer.measure(
                text = AnnotatedString(lines.size.toString()),
                style = lineNumberStyle,
            ).size.width.toFloat()
            return CodeViewMetrics(
                lineHeight = lineHeight,
                charWidth = charWidth,
                gutterWidth = max(gutterWidth, lineNumberWidth + LINE_NUMBER_PADDING * 2f),
                codeLeftPadding = CODE_LEFT_PADDING,
                verticalPadding = VERTICAL_PADDING,
                width = ceil(gutterWidth + CODE_LEFT_PADDING + maxLineWidth + 28f),
                height = ceil(lines.size * lineHeight + VERTICAL_PADDING * 2f),
            )
        }
    }
}
