package com.qiplat.sweetline;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

import static java.lang.foreign.ValueLayout.JAVA_INT;

/**
 * Plain text highlight analyzer, no incremental update support.
 * Suitable for full text analysis or single-line analysis scenarios.
 * <p>
 * Implements {@link AutoCloseable} for deterministic native resource release.
 */
public class TextAnalyzer implements AutoCloseable {

    private final MemorySegment handle;
    private boolean closed = false;

    TextAnalyzer(MemorySegment handle) {
        this.handle = handle;
    }

    /**
     * Analyze a text and return the highlight result for the entire text.
     *
     * @param text Full text content
     * @return Highlight result
     */
    public DocumentHighlight analyzeText(String text) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment textSeg = arena.allocateFrom(text);
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_text_analyze.invoke(handle, textSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new DocumentHighlight();
            }
            try {
                return BufferParser.readDocumentHighlight(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze text", e);
        }
    }

    /**
     * Analyze a single line of text.
     *
     * @param text Single line text content
     * @param info Metadata for the current line
     * @return Single line analysis result
     */
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment textSeg = arena.allocateFrom(text);
            // Pack TextLineInfo into int32_t[3]
            MemorySegment lineInfoSeg = arena.allocate(JAVA_INT, 3);
            lineInfoSeg.setAtIndex(JAVA_INT, 0, info.line());
            lineInfoSeg.setAtIndex(JAVA_INT, 1, info.startState());
            lineInfoSeg.setAtIndex(JAVA_INT, 2, info.startCharOffset());

            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_text_analyze_line
                    .invoke(handle, textSeg, lineInfoSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new LineAnalyzeResult(new LineHighlight(), 0, 0);
            }
            try {
                return BufferParser.readLineAnalyzeResult(resultPtr, info.line());
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze line", e);
        }
    }

    /**
     * Perform indent guide analysis on a text.
     *
     * @param text Full text content
     * @return Indent guide analysis result
     */
    public IndentGuideResult analyzeIndentGuides(String text) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment textSeg = arena.allocateFrom(text);
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_text_analyze_indent_guides
                    .invoke(handle, textSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new IndentGuideResult();
            }
            try {
                return BufferParser.readIndentGuideResult(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze indent guides", e);
        }
    }

    @Override
    public void close() {
        if (!closed) {
            closed = true;
            // TextAnalyzer handle is managed by the engine internally;
            // the C API does not expose a separate free function for text analyzer.
            // The handle is freed when the engine is destroyed.
        }
    }

    private void ensureOpen() {
        if (closed) {
            throw new IllegalStateException("TextAnalyzer is already closed");
        }
    }
}
