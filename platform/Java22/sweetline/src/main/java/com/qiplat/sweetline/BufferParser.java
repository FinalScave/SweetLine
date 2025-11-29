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
     * buffer[0] = spanCount
     * buffer[1] = stride
     * Followed by spanCount * stride int32_t fields
     * </pre>
     */
    static DocumentHighlight readDocumentHighlight(MemorySegment bufferPtr) {
        DocumentHighlight highlight = new DocumentHighlight();
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return highlight;
        }

        // Read header: [spanCount, stride]
        MemorySegment header = bufferPtr.reinterpret(2L * JAVA_INT.byteSize());
        int spanCount = header.getAtIndex(JAVA_INT, 0);
        int stride = header.getAtIndex(JAVA_INT, 1);

        if (spanCount <= 0 || stride <= 0) {
            return highlight;
        }

        // Reinterpret to full buffer size
        long totalInts = 2L + (long) spanCount * stride;
        MemorySegment buffer = bufferPtr.reinterpret(totalInts * JAVA_INT.byteSize());

        LineHighlight lineHighlight = new LineHighlight();
        int currentLine = -1;

        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 2;
            int startLine = buffer.getAtIndex(JAVA_INT, baseIndex);
            int startColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 1);
            int startIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 2);
            int endLine = buffer.getAtIndex(JAVA_INT, baseIndex + 3);
            int endColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 4);
            int endIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 5);

            TokenSpan span = readTokenSpan(buffer, baseIndex, stride,
                    startLine, startColumn, startIndex, endLine, endColumn, endIndex);

            if (startLine != currentLine) {
                currentLine = startLine;
                lineHighlight = new LineHighlight();
                highlight.lines().add(lineHighlight);
            }
            lineHighlight.spans().add(span);
        }
        return highlight;
    }

    /**
     * Parse highlight slice for specified line range from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = startLine
     * buffer[1] = totalLineCount
     * buffer[2] = lineCount
     * buffer[3] = spanCount
     * buffer[4] = stride
     * Followed by spanCount * stride int32_t fields
     * </pre>
     */
    static DocumentHighlightSlice readDocumentHighlightSlice(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new DocumentHighlightSlice(0, 0, new ArrayList<>());
        }

        MemorySegment header = bufferPtr.reinterpret(5L * JAVA_INT.byteSize());
        int startLine = header.getAtIndex(JAVA_INT, 0);
        int totalLineCount = header.getAtIndex(JAVA_INT, 1);
        int lineCount = Math.max(header.getAtIndex(JAVA_INT, 2), 0);
        int spanCount = Math.max(header.getAtIndex(JAVA_INT, 3), 0);
        int stride = Math.max(header.getAtIndex(JAVA_INT, 4), 0);

        long totalInts = 5L + (long) spanCount * stride;
        MemorySegment buffer = bufferPtr.reinterpret(totalInts * JAVA_INT.byteSize());

        List<LineHighlight> lines = new ArrayList<>(lineCount);
        for (int i = 0; i < lineCount; i++) {
            lines.add(new LineHighlight());
        }

        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 5;
            int spanStartLine = buffer.getAtIndex(JAVA_INT, baseIndex);
            int spanStartColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 1);
            int spanStartIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 2);
            int spanEndLine = buffer.getAtIndex(JAVA_INT, baseIndex + 3);
            int spanEndColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 4);
            int spanEndIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 5);

            TokenSpan span = readTokenSpan(buffer, baseIndex, stride,
                    spanStartLine, spanStartColumn, spanStartIndex, spanEndLine, spanEndColumn, spanEndIndex);

            int localLine = spanStartLine - startLine;
            if (localLine >= 0 && localLine < lines.size()) {
                lines.get(localLine).spans().add(span);
            }
        }
        return new DocumentHighlightSlice(startLine, totalLineCount, lines);
    }

    /**
     * Parse single line analysis result from native buffer.
     * <p>Layout:
     * <pre>
     * buffer[0] = spanCount
     * buffer[1] = stride
     * buffer[2] = endState
     * buffer[3] = charCount
     * Followed by spanCount * stride int32_t fields
     * </pre>
     */
    static LineAnalyzeResult readLineAnalyzeResult(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new LineAnalyzeResult(new LineHighlight(), 0, 0);
        }

        MemorySegment header = bufferPtr.reinterpret(4L * JAVA_INT.byteSize());
        int spanCount = header.getAtIndex(JAVA_INT, 0);
        int stride = header.getAtIndex(JAVA_INT, 1);
        int endState = header.getAtIndex(JAVA_INT, 2);
        int charCount = header.getAtIndex(JAVA_INT, 3);

        long totalInts = 4L + (long) spanCount * stride;
        MemorySegment buffer = bufferPtr.reinterpret(totalInts * JAVA_INT.byteSize());

        LineHighlight lineHighlight = new LineHighlight();
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 4;
            int startLine = buffer.getAtIndex(JAVA_INT, baseIndex);
            int startColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 1);
            int startIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 2);
            int endLine = buffer.getAtIndex(JAVA_INT, baseIndex + 3);
            int endColumn = buffer.getAtIndex(JAVA_INT, baseIndex + 4);
            int endIndex = buffer.getAtIndex(JAVA_INT, baseIndex + 5);

            TokenSpan span = readTokenSpan(buffer, baseIndex, stride,
                    startLine, startColumn, startIndex, endLine, endColumn, endIndex);
            lineHighlight.spans().add(span);
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

        // Estimate total buffer size: header(4) + guides(at least 6 each) + lineStates(4 each)
        // Use a generous estimate to reinterpret, then read sequentially
        long estimatedSize = 4L + (long) guideCount * 6 + (long) lineStateCount * 4;
        // The actual size may be larger due to branches, but we'll reinterpret conservatively
        // and then expand as needed. For safety, use a large reinterpret.
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

    private static TokenSpan readTokenSpan(MemorySegment buffer, int baseIndex, int stride,
                                           int startLine, int startColumn, int startIndex,
                                           int endLine, int endColumn, int endIndex) {
        TextRange range = new TextRange(
                new TextPosition(startLine, startColumn, startIndex),
                new TextPosition(endLine, endColumn, endIndex)
        );

        if (stride > 7) {
            // Inline style mode: [fg, bg, fontAttributes]
            int fg = buffer.getAtIndex(JAVA_INT, baseIndex + 6);
            int bg = buffer.getAtIndex(JAVA_INT, baseIndex + 7);
            int fontAttr = buffer.getAtIndex(JAVA_INT, baseIndex + 8);
            return new TokenSpan(range, new InlineStyle(fg, bg, fontAttr));
        } else {
            // Style ID mode: [styleId]
            int styleId = buffer.getAtIndex(JAVA_INT, baseIndex + 6);
            return new TokenSpan(range, styleId);
        }
    }
}
