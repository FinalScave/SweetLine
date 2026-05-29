package com.qiplat.sweetline

internal object NativeBufferParser {
    fun readDocumentHighlight(buffer: IntArray?): DocumentHighlight {
        if (buffer == null || buffer.size < 3) {
            return DocumentHighlight()
        }
        return readDocumentHighlight { index -> buffer.getOrElse(index) { 0 } }
    }

    fun readDocumentHighlight(read: (Int) -> Int): DocumentHighlight {
        val flags = read(0)
        val stride = read(1).coerceAtLeast(0)
        val lineCount = read(2).coerceAtLeast(0)
        val hasStartIndex = flagsHasStartIndex(flags)
        val inlineStyle = flagsUsesInlineStyle(flags)
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return DocumentHighlight()
        }

        val lines = ArrayList<LineHighlight>(lineCount)
        var index = 3
        repeat(lineCount) { line ->
            val spans = ArrayList<TokenSpan>()
            val spanCount = read(index++).coerceAtLeast(0)
            repeat(spanCount) {
                val startColumn = read(index++)
                val length = read(index++)
                val startIndex = if (hasStartIndex) read(index++) else 0
                val range = TextRange(
                    start = TextPosition(line, startColumn, startIndex),
                    end = TextPosition(
                        line = line,
                        column = startColumn + length,
                        index = if (hasStartIndex) startIndex + length else 0,
                    ),
                )
                spans += if (inlineStyle) {
                    TokenSpan(
                        range = range,
                        inlineStyle = InlineStyle(
                            foreground = read(index++),
                            background = read(index++),
                            fontAttributes = read(index++),
                        ),
                    )
                } else {
                    TokenSpan(range = range, styleId = read(index++))
                }
            }
            lines += LineHighlight(spans)
        }
        return DocumentHighlight(lines)
    }

    fun readDocumentHighlightSlice(buffer: IntArray?): DocumentHighlightSlice {
        if (buffer == null || buffer.size < 5) {
            return DocumentHighlightSlice()
        }
        return readDocumentHighlightSlice { index -> buffer.getOrElse(index) { 0 } }
    }

    fun readDocumentHighlightSlice(read: (Int) -> Int): DocumentHighlightSlice {
        val flags = read(0)
        val stride = read(1).coerceAtLeast(0)
        val startLine = read(2)
        val totalLineCount = read(3)
        val lineCount = read(4).coerceAtLeast(0)
        val hasStartIndex = flagsHasStartIndex(flags)
        val inlineStyle = flagsUsesInlineStyle(flags)
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return DocumentHighlightSlice(startLine = startLine, totalLineCount = totalLineCount)
        }

        val lines = ArrayList<LineHighlight>(lineCount)
        var index = 5
        repeat(lineCount) { offset ->
            val line = startLine + offset
            val spans = ArrayList<TokenSpan>()
            val spanCount = read(index++).coerceAtLeast(0)
            repeat(spanCount) {
                val startColumn = read(index++)
                val length = read(index++)
                val startIndex = if (hasStartIndex) read(index++) else 0
                val range = TextRange(
                    start = TextPosition(line, startColumn, startIndex),
                    end = TextPosition(
                        line = line,
                        column = startColumn + length,
                        index = if (hasStartIndex) startIndex + length else 0,
                    ),
                )
                spans += if (inlineStyle) {
                    TokenSpan(
                        range = range,
                        inlineStyle = InlineStyle(
                            foreground = read(index++),
                            background = read(index++),
                            fontAttributes = read(index++),
                        ),
                    )
                } else {
                    TokenSpan(range = range, styleId = read(index++))
                }
            }
            lines += LineHighlight(spans)
        }
        return DocumentHighlightSlice(startLine, totalLineCount, lines)
    }

    fun readLineAnalyzeResult(buffer: IntArray?, lineNumber: Int): LineAnalyzeResult {
        if (buffer == null || buffer.size < 5) {
            return LineAnalyzeResult()
        }
        return readLineAnalyzeResult(lineNumber) { index -> buffer.getOrElse(index) { 0 } }
    }

    fun readLineAnalyzeResult(lineNumber: Int, read: (Int) -> Int): LineAnalyzeResult {
        val flags = read(0)
        val stride = read(1)
        val spanCount = read(2).coerceAtLeast(0)
        val endState = read(3)
        val charCount = read(4)
        val hasStartIndex = flagsHasStartIndex(flags)
        val inlineStyle = flagsUsesInlineStyle(flags)
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return LineAnalyzeResult(endState = endState, charCount = charCount)
        }

        val spans = ArrayList<TokenSpan>(spanCount)
        var index = 5
        repeat(spanCount) {
            val startColumn = read(index++)
            val length = read(index++)
            val startIndex = if (hasStartIndex) read(index++) else 0
            val range = TextRange(
                start = TextPosition(lineNumber, startColumn, startIndex),
                end = TextPosition(
                    line = lineNumber,
                    column = startColumn + length,
                    index = if (hasStartIndex) startIndex + length else 0,
                ),
            )
            spans += if (inlineStyle) {
                TokenSpan(
                    range = range,
                    inlineStyle = InlineStyle(
                        foreground = read(index++),
                        background = read(index++),
                        fontAttributes = read(index++),
                    ),
                )
            } else {
                TokenSpan(range = range, styleId = read(index++))
            }
        }
        return LineAnalyzeResult(LineHighlight(spans), endState, charCount)
    }

    fun readIndentGuideResult(buffer: IntArray?): IndentGuideResult {
        if (buffer == null || buffer.size < 3) {
            return IndentGuideResult()
        }
        return readIndentGuideResult { index -> buffer.getOrElse(index) { 0 } }
    }

    fun readIndentGuideResult(read: (Int) -> Int): IndentGuideResult {
        val startLine = read(0)
        val lineStateCount = read(1).coerceAtLeast(0)
        val guideCount = read(2).coerceAtLeast(0)
        val guideLines = ArrayList<IndentGuideLine>(guideCount)
        val lineStates = ArrayList<LineScopeState>(lineStateCount)
        var index = 3

        repeat(guideCount) {
            val column = read(index++)
            val startLine = read(index++)
            val endLine = read(index++)
            val flags = read(index++)
            val continuesBefore = (flags and 1) != 0
            val continuesAfter = (flags and (1 shl 1)) != 0
            val branchCount = read(index++).coerceAtLeast(0)
            val branches = ArrayList<IndentGuideLine.BranchPoint>(branchCount)
            repeat(branchCount) {
                branches += IndentGuideLine.BranchPoint(
                    line = read(index++),
                    column = read(index++),
                )
            }
            guideLines += IndentGuideLine(
                column = column,
                startLine = startLine,
                endLine = endLine,
                continuesBefore = continuesBefore,
                continuesAfter = continuesAfter,
                branches = branches,
            )
        }

        repeat(lineStateCount) {
            lineStates += LineScopeState(
                nestingLevel = read(index++),
                scopeState = read(index++),
                scopeColumn = read(index++),
                indentLevel = read(index++),
            )
        }
        return IndentGuideResult(startLine, guideLines, lineStates)
    }

    private fun isValidSpanStride(stride: Int, hasStartIndex: Boolean, inlineStyle: Boolean): Boolean {
        val expected = 2 + (if (hasStartIndex) 1 else 0) + (if (inlineStyle) 3 else 1)
        return stride == expected
    }

    private fun flagsUsesInlineStyle(flags: Int): Boolean {
        return (flags and (1 shl 1)) != 0
    }

    private fun flagsHasStartIndex(flags: Int): Boolean {
        return (flags and 1) != 0
    }
}
