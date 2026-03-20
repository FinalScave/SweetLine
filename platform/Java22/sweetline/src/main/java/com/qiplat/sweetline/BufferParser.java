package com.qiplat.sweetline;

import java.lang.foreign.MemorySegment;
import java.util.ArrayList;
import java.util.List;

import static java.lang.foreign.ValueLayout.JAVA_INT;

/**
 * Parses int32_t* buffers returned by the SweetLine C API into Java data objects.
 * All parsing methods expect a non-null MemorySegment pointing to the native buffer.
 */
final class BufferParser {

    private BufferParser() {
    }

    /**
     * Parse full document highlight from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = flags
     * buffer[1] = stride
     * buffer[2] = lineCount
     * Followed by lineCount line entries:
     * lineEntry[0] = spanCount of current line
     * followed by spanCount * stride int32_t fields
     * </pre>
     */
    static DocumentHighlight readDocumentHighlight(MemorySegment bufferPtr) {
        DocumentHighlight highlight = new DocumentHighlight();
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return highlight;
        }

        // Read header: [flags, stride, lineCount]
        MemorySegment header = bufferPtr.reinterpret(3L * JAVA_INT.byteSize());
        int flags = header.getAtIndex(JAVA_INT, 0);
        int stride = header.getAtIndex(JAVA_INT, 1);
        int lineCount = Math.max(header.getAtIndex(JAVA_INT, 2), 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean inlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return highlight;
        }
        MemorySegment buffer = bufferPtr.reinterpret(Long.MAX_VALUE);

        int idx = 3;
        for (int line = 0; line < lineCount; line++) {
            LineHighlight lineHighlight = new LineHighlight();
            highlight.lines().add(lineHighlight);
            int spanCount = Math.max(buffer.getAtIndex(JAVA_INT, idx++), 0);
            for (int i = 0; i < spanCount; i++) {
                int startColumn = buffer.getAtIndex(JAVA_INT, idx++);
                int length = buffer.getAtIndex(JAVA_INT, idx++);
                int startIndex = hasStartIndex ? buffer.getAtIndex(JAVA_INT, idx++) : 0;
                int endColumn = startColumn + length;
                int endIndex = hasStartIndex ? startIndex + length : 0;
                TextRange range = new TextRange(
                        new TextPosition(line, startColumn, startIndex),
                        new TextPosition(line, endColumn, endIndex)
                );
                if (inlineStyle) {
                    int fg = buffer.getAtIndex(JAVA_INT, idx++);
                    int bg = buffer.getAtIndex(JAVA_INT, idx++);
                    int fontAttr = buffer.getAtIndex(JAVA_INT, idx++);
                    lineHighlight.spans().add(new TokenSpan(range, new InlineStyle(fg, bg, fontAttr)));
                } else {
                    int styleId = buffer.getAtIndex(JAVA_INT, idx++);
                    lineHighlight.spans().add(new TokenSpan(range, styleId));
                }
            }
        }
        return highlight;
    }

    /**
     * Parse highlight slice for specified line range from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = flags
     * buffer[1] = stride
     * buffer[2] = startLine
     * buffer[3] = totalLineCount
     * buffer[4] = lineCount
     * Followed by lineCount line entries:
     * lineEntry[0] = spanCount of current line
     * followed by spanCount * stride int32_t fields
     * </pre>
     */
    static DocumentHighlightSlice readDocumentHighlightSlice(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new DocumentHighlightSlice(0, 0, new ArrayList<>());
        }

        MemorySegment header = bufferPtr.reinterpret(5L * JAVA_INT.byteSize());
        int flags = header.getAtIndex(JAVA_INT, 0);
        int stride = Math.max(header.getAtIndex(JAVA_INT, 1), 0);
        int startLine = header.getAtIndex(JAVA_INT, 2);
        int totalLineCount = header.getAtIndex(JAVA_INT, 3);
        int lineCount = Math.max(header.getAtIndex(JAVA_INT, 4), 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean inlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return new DocumentHighlightSlice(startLine, totalLineCount, new ArrayList<>());
        }
        MemorySegment buffer = bufferPtr.reinterpret(Long.MAX_VALUE);

        List<LineHighlight> lines = new ArrayList<>(lineCount);
        for (int i = 0; i < lineCount; i++) {
            lines.add(new LineHighlight());
        }

        int idx = 5;
        for (int i = 0; i < lineCount; i++) {
            int spanCount = Math.max(buffer.getAtIndex(JAVA_INT, idx++), 0);
            int line = startLine + i;
            LineHighlight lineHighlight = lines.get(i);
            for (int s = 0; s < spanCount; s++) {
                int startColumn = buffer.getAtIndex(JAVA_INT, idx++);
                int length = buffer.getAtIndex(JAVA_INT, idx++);
                int startIndex = hasStartIndex ? buffer.getAtIndex(JAVA_INT, idx++) : 0;
                int endColumn = startColumn + length;
                int endIndex = hasStartIndex ? startIndex + length : 0;
                TextRange range = new TextRange(
                        new TextPosition(line, startColumn, startIndex),
                        new TextPosition(line, endColumn, endIndex)
                );
                if (inlineStyle) {
                    int fg = buffer.getAtIndex(JAVA_INT, idx++);
                    int bg = buffer.getAtIndex(JAVA_INT, idx++);
                    int fontAttr = buffer.getAtIndex(JAVA_INT, idx++);
                    lineHighlight.spans().add(new TokenSpan(range, new InlineStyle(fg, bg, fontAttr)));
                } else {
                    int styleId = buffer.getAtIndex(JAVA_INT, idx++);
                    lineHighlight.spans().add(new TokenSpan(range, styleId));
                }
            }
        }
        return new DocumentHighlightSlice(startLine, totalLineCount, lines);
    }

    /**
     * Parse single line analysis result from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = flags
     * buffer[1] = stride
     * buffer[2] = spanCount
     * buffer[3] = endState
     * buffer[4] = charCount
     * Followed by spanCount * stride int32_t fields
     * </pre>
     */
    static LineAnalyzeResult readLineAnalyzeResult(MemorySegment bufferPtr) {
        return readLineAnalyzeResult(bufferPtr, 0);
    }

    static LineAnalyzeResult readLineAnalyzeResult(MemorySegment bufferPtr, int lineNumber) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new LineAnalyzeResult(new LineHighlight(), 0, 0);
        }

        MemorySegment header = bufferPtr.reinterpret(5L * JAVA_INT.byteSize());
        int flags = header.getAtIndex(JAVA_INT, 0);
        int stride = header.getAtIndex(JAVA_INT, 1);
        int spanCount = header.getAtIndex(JAVA_INT, 2);
        int endState = header.getAtIndex(JAVA_INT, 3);
        int charCount = header.getAtIndex(JAVA_INT, 4);

        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean inlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
            return new LineAnalyzeResult(new LineHighlight(), endState, charCount);
        }
        MemorySegment buffer = bufferPtr.reinterpret(Long.MAX_VALUE);

        LineHighlight lineHighlight = new LineHighlight();
        int idx = 5;
        for (int i = 0; i < spanCount; i++) {
            int startColumn = buffer.getAtIndex(JAVA_INT, idx++);
            int length = buffer.getAtIndex(JAVA_INT, idx++);
            int startIndex = hasStartIndex ? buffer.getAtIndex(JAVA_INT, idx++) : 0;
            int endColumn = startColumn + length;
            int endIndex = hasStartIndex ? startIndex + length : 0;

            TextRange range = new TextRange(
                    new TextPosition(lineNumber, startColumn, startIndex),
                    new TextPosition(lineNumber, endColumn, endIndex)
            );

            if (inlineStyle) {
                int fg = buffer.getAtIndex(JAVA_INT, idx++);
                int bg = buffer.getAtIndex(JAVA_INT, idx++);
                int fontAttr = buffer.getAtIndex(JAVA_INT, idx++);
                lineHighlight.spans().add(new TokenSpan(range, new InlineStyle(fg, bg, fontAttr)));
            } else {
                int styleId = buffer.getAtIndex(JAVA_INT, idx++);
                lineHighlight.spans().add(new TokenSpan(range, styleId));
            }
        }
        return new LineAnalyzeResult(lineHighlight, endState, charCount);
    }

    /**
     * Parse indent guide analysis result from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = guide_lines count
     * buffer[1] = fixed field count per guide_line (stride=6)
     * buffer[2] = line_states count
     * buffer[3] = field count per line_state (4)
     * Followed by guide_lines data (variable length due to branches)
     * Followed by line_states data
     * </pre>
     */
    static IndentGuideResult readIndentGuideResult(MemorySegment bufferPtr) {
        IndentGuideResult result = new IndentGuideResult();
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return result;
        }

        MemorySegment header = bufferPtr.reinterpret(4L * JAVA_INT.byteSize());
        int guideCount = header.getAtIndex(JAVA_INT, 0);
        // buffer[1] = stride (unused, we read dynamically due to variable branch count)
        int lineStateCount = header.getAtIndex(JAVA_INT, 2);
        // buffer[3] = lineStateStride (always 4)

        // The actual size may be larger due to branches, use a generous reinterpret upper bound.
        // A better approach: first scan guide lines to compute real size.
        // Since we need sequential access, reinterpret with a generous upper bound.
        long maxSize = 4L + (long) guideCount * 1024 + (long) lineStateCount * 4; // generous
        MemorySegment buffer = bufferPtr.reinterpret(maxSize * JAVA_INT.byteSize());

        int idx = 4;
        for (int i = 0; i < guideCount; i++) {
            int column = buffer.getAtIndex(JAVA_INT, idx++);
            int guidStartLine = buffer.getAtIndex(JAVA_INT, idx++);
            int guidEndLine = buffer.getAtIndex(JAVA_INT, idx++);
            int nestingLevel = buffer.getAtIndex(JAVA_INT, idx++);
            int scopeRuleId = buffer.getAtIndex(JAVA_INT, idx++);
            int branchCount = buffer.getAtIndex(JAVA_INT, idx++);

            IndentGuideLine guide = new IndentGuideLine(column, guidStartLine, guidEndLine, nestingLevel, scopeRuleId);
            for (int j = 0; j < branchCount; j++) {
                int bLine = buffer.getAtIndex(JAVA_INT, idx++);
                int bColumn = buffer.getAtIndex(JAVA_INT, idx++);
                guide.branches().add(new IndentGuideLine.BranchPoint(bLine, bColumn));
            }
            result.guideLines().add(guide);
        }

        for (int i = 0; i < lineStateCount; i++) {
            int nestingLevel = buffer.getAtIndex(JAVA_INT, idx++);
            int scopeState = buffer.getAtIndex(JAVA_INT, idx++);
            int scopeColumn = buffer.getAtIndex(JAVA_INT, idx++);
            int indentLevel = buffer.getAtIndex(JAVA_INT, idx++);
            result.lineStates().add(new LineScopeState(nestingLevel, scopeState, scopeColumn, indentLevel));
        }

        return result;
    }

    private static boolean isValidSpanStride(int stride, boolean hasStartIndex, boolean inlineStyle) {
        int expected = 2 + (hasStartIndex ? 1 : 0) + (inlineStyle ? 3 : 1);
        return stride == expected;
    }

    private static boolean flagsUsesInlineStyle(int flags) {
        return (flags & (1 << 1)) != 0;
    }

    private static boolean flagsHasStartIndex(int flags) {
        return (flags & 1) != 0;
    }
}
