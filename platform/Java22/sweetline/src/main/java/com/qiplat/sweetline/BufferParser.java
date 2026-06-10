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
     * buffer[0] = start line
     * buffer[1] = line state count
     * buffer[2] = guide line count
     * Followed by guide_lines data (variable length due to branches)
     * Followed by line_states data
     * </pre>
     */
    static IndentGuideResult readIndentGuideResult(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new IndentGuideResult();
        }

        MemorySegment header = bufferPtr.reinterpret(3L * JAVA_INT.byteSize());
        int startLine = header.getAtIndex(JAVA_INT, 0);
        int lineStateCount = Math.max(header.getAtIndex(JAVA_INT, 1), 0);
        int guideCount = Math.max(header.getAtIndex(JAVA_INT, 2), 0);
        IndentGuideResult result = new IndentGuideResult(startLine, new ArrayList<>(), new ArrayList<>());

        // The actual size may be larger due to branches, use a generous reinterpret upper bound.
        // A better approach: first scan guide lines to compute real size.
        // Since we need sequential access, reinterpret with a generous upper bound.
        long maxSize = 3L + (long) guideCount * 1024 + (long) lineStateCount * 4; // generous
        MemorySegment buffer = bufferPtr.reinterpret(maxSize * JAVA_INT.byteSize());

        int idx = 3;
        for (int i = 0; i < guideCount; i++) {
            int column = buffer.getAtIndex(JAVA_INT, idx++);
            int guidStartLine = buffer.getAtIndex(JAVA_INT, idx++);
            int guidEndLine = buffer.getAtIndex(JAVA_INT, idx++);
            int flags = buffer.getAtIndex(JAVA_INT, idx++);
            boolean continuesBefore = (flags & 1) != 0;
            boolean continuesAfter = (flags & (1 << 1)) != 0;
            int branchCount = Math.max(buffer.getAtIndex(JAVA_INT, idx++), 0);

            IndentGuideLine guide = new IndentGuideLine(column, guidStartLine, guidEndLine, continuesBefore, continuesAfter);
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

    static BracketPairResult readBracketPairResult(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new BracketPairResult();
        }

        MemorySegment header = bufferPtr.reinterpret(3L * JAVA_INT.byteSize());
        int flags = header.getAtIndex(JAVA_INT, 0);
        int stride = Math.max(header.getAtIndex(JAVA_INT, 1), 0);
        int lineCount = Math.max(header.getAtIndex(JAVA_INT, 2), 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        if (!isValidBracketTokenStride(stride, hasStartIndex)) {
            return new BracketPairResult();
        }
        List<LineBracketPairs> lines = readBracketLines(bufferPtr, 3, 0, lineCount, hasStartIndex);
        return new BracketPairResult(0, lineCount, lines);
    }

    static BracketPairResult readBracketPairResultSlice(MemorySegment bufferPtr) {
        if (bufferPtr.equals(MemorySegment.NULL)) {
            return new BracketPairResult();
        }

        MemorySegment header = bufferPtr.reinterpret(5L * JAVA_INT.byteSize());
        int flags = header.getAtIndex(JAVA_INT, 0);
        int stride = Math.max(header.getAtIndex(JAVA_INT, 1), 0);
        int startLine = header.getAtIndex(JAVA_INT, 2);
        int totalLineCount = header.getAtIndex(JAVA_INT, 3);
        int lineCount = Math.max(header.getAtIndex(JAVA_INT, 4), 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        if (!isValidBracketTokenStride(stride, hasStartIndex)) {
            return new BracketPairResult(startLine, totalLineCount, new ArrayList<>());
        }
        List<LineBracketPairs> lines = readBracketLines(bufferPtr, 5, startLine, lineCount, hasStartIndex);
        return new BracketPairResult(startLine, totalLineCount, lines);
    }

    private static List<LineBracketPairs> readBracketLines(
            MemorySegment bufferPtr,
            int startIndex,
            int startLine,
            int lineCount,
            boolean hasStartIndex) {
        MemorySegment buffer = bufferPtr.reinterpret(Long.MAX_VALUE);
        List<LineBracketPairs> lines = new ArrayList<>(lineCount);
        int idx = startIndex;
        for (int i = 0; i < lineCount; i++) {
            int tokenCount = Math.max(buffer.getAtIndex(JAVA_INT, idx++), 0);
            int line = startLine + i;
            LineBracketPairs lineResult = new LineBracketPairs();
            for (int token = 0; token < tokenCount; token++) {
                int column = buffer.getAtIndex(JAVA_INT, idx++);
                int length = buffer.getAtIndex(JAVA_INT, idx++);
                int tokenStartIndex = hasStartIndex ? buffer.getAtIndex(JAVA_INT, idx++) : 0;
                int depth = buffer.getAtIndex(JAVA_INT, idx++);
                int kind = buffer.getAtIndex(JAVA_INT, idx++);
                int matchState = buffer.getAtIndex(JAVA_INT, idx++);
                int partnerLine = buffer.getAtIndex(JAVA_INT, idx++);
                int partnerColumn = buffer.getAtIndex(JAVA_INT, idx++);
                int partnerLength = buffer.getAtIndex(JAVA_INT, idx++);
                int partnerStartIndex = hasStartIndex ? buffer.getAtIndex(JAVA_INT, idx++) : 0;

                TextRange range = new TextRange(
                        new TextPosition(line, column, tokenStartIndex),
                        new TextPosition(line, column + length, hasStartIndex ? tokenStartIndex + length : 0)
                );
                TextRange partnerRange = null;
                if (partnerLine >= 0 && partnerColumn >= 0 && partnerLength >= 0) {
                    partnerRange = new TextRange(
                            new TextPosition(partnerLine, partnerColumn, partnerStartIndex),
                            new TextPosition(partnerLine, partnerColumn + partnerLength,
                                    hasStartIndex ? partnerStartIndex + partnerLength : 0)
                    );
                }
                lineResult.tokens().add(new BracketToken(
                        range,
                        depth,
                        BracketTokenKind.fromValue(kind),
                        BracketMatchState.fromValue(matchState),
                        partnerRange
                ));
            }
            lines.add(lineResult);
        }
        return lines;
    }

    private static boolean isValidSpanStride(int stride, boolean hasStartIndex, boolean inlineStyle) {
        int expected = 2 + (hasStartIndex ? 1 : 0) + (inlineStyle ? 3 : 1);
        return stride == expected;
    }

    private static boolean isValidBracketTokenStride(int stride, boolean hasStartIndex) {
        int expected = 8 + (hasStartIndex ? 2 : 0);
        return stride == expected;
    }

    private static boolean flagsUsesInlineStyle(int flags) {
        return (flags & (1 << 1)) != 0;
    }

    private static boolean flagsHasStartIndex(int flags) {
        return (flags & 1) != 0;
    }
}
