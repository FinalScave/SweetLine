package com.qiplat.sweetline;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.util.ArrayList;

import static java.lang.foreign.ValueLayout.JAVA_INT;

/**
 * Document highlight analyzer with incremental update support.
 * <p>
 * Created via {@link HighlightEngine#loadDocument(Document)}.
 * Supports incremental re-analysis when document content changes.
 * Implements {@link AutoCloseable} for deterministic native resource release.
 */
public class DocumentAnalyzer implements AutoCloseable {

    private final MemorySegment handle;
    private boolean closed = false;

    DocumentAnalyzer(MemorySegment handle) {
        this.handle = handle;
    }

    /**
     * Perform full highlight analysis on the managed document.
     * Typically called once after initial document load.
     *
     * @return Highlight result for the entire document
     */
    public DocumentHighlight analyze() {
        ensureOpen();
        try {
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_document_analyze.invoke(handle);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new DocumentHighlight();
            }
            try {
                return BufferParser.readDocumentHighlight(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze document", e);
        }
    }

    /**
     * Incrementally re-analyze based on a text change, returning highlight for the entire document.
     *
     * @param range   Change range (start/end line and column)
     * @param newText The new text replacing the given range
     * @return Highlight result for the entire document
     */
    public DocumentHighlight analyzeIncremental(TextRange range, String newText) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            // Pack changes_range: [startLine, startColumn, endLine, endColumn]
            MemorySegment changesSeg = arena.allocate(JAVA_INT, 4);
            changesSeg.setAtIndex(JAVA_INT, 0, range.start().line());
            changesSeg.setAtIndex(JAVA_INT, 1, range.start().column());
            changesSeg.setAtIndex(JAVA_INT, 2, range.end().line());
            changesSeg.setAtIndex(JAVA_INT, 3, range.end().column());

            MemorySegment textSeg = arena.allocateFrom(newText);
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_document_analyze_incremental
                    .invoke(handle, changesSeg, textSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new DocumentHighlight();
            }
            try {
                return BufferParser.readDocumentHighlight(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze document incrementally", e);
        }
    }

    /**
     * Incrementally re-analyze and return highlight slice for the specified visible line range.
     *
     * @param range        Change range (start/end line and column)
     * @param newText      The new text replacing the given range
     * @param visibleRange Visible line range (startLine + lineCount)
     * @return Highlight slice for the specified line range
     */
    public DocumentHighlightSlice analyzeIncrementalInLineRange(TextRange range, String newText, LineRange visibleRange) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            // Pack changes_range: [startLine, startColumn, endLine, endColumn]
            MemorySegment changesSeg = arena.allocate(JAVA_INT, 4);
            changesSeg.setAtIndex(JAVA_INT, 0, range.start().line());
            changesSeg.setAtIndex(JAVA_INT, 1, range.start().column());
            changesSeg.setAtIndex(JAVA_INT, 2, range.end().line());
            changesSeg.setAtIndex(JAVA_INT, 3, range.end().column());

            // Pack visible_range: [startLine, lineCount]
            MemorySegment visibleSeg = arena.allocate(JAVA_INT, 2);
            visibleSeg.setAtIndex(JAVA_INT, 0, visibleRange.startLine());
            visibleSeg.setAtIndex(JAVA_INT, 1, visibleRange.lineCount());

            MemorySegment textSeg = arena.allocateFrom(newText);
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_document_analyze_incremental_in_line_range
                    .invoke(handle, changesSeg, textSeg, visibleSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new DocumentHighlightSlice(0, 0, new ArrayList<>());
            }
            try {
                return BufferParser.readDocumentHighlightSlice(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to analyze document incrementally in line range", e);
        }
    }

    /**
     * Get highlight slice from the current cached result.
     * Requires prior call to {@link #analyze()} or {@link #analyzeIncremental(TextRange, String)}.
     *
     * @param visibleRange Visible line range (startLine + lineCount)
     * @return Highlight slice for the specified line range
     */
    public DocumentHighlightSlice getHighlightSlice(LineRange visibleRange) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment visibleSeg = arena.allocate(JAVA_INT, 2);
            visibleSeg.setAtIndex(JAVA_INT, 0, visibleRange.startLine());
            visibleSeg.setAtIndex(JAVA_INT, 1, visibleRange.lineCount());

            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_document_get_highlight_slice
                    .invoke(handle, visibleSeg);
            if (resultPtr.equals(MemorySegment.NULL)) {
                return new DocumentHighlightSlice(0, 0, new ArrayList<>());
            }
            try {
                return BufferParser.readDocumentHighlightSlice(resultPtr);
            } finally {
                SweetLineNative.sl_free_buffer.invoke(resultPtr);
            }
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get highlight slice", e);
        }
    }

    /**
     * Perform indent guide analysis on the managed document.
     * Requires prior call to {@link #analyze()} or {@link #analyzeIncremental(TextRange, String)}.
     *
     * @return Indent guide analysis result
     */
    public IndentGuideResult analyzeIndentGuides() {
        ensureOpen();
        try {
            MemorySegment resultPtr = (MemorySegment) SweetLineNative.sl_document_analyze_indent_guides.invoke(handle);
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
            // DocumentAnalyzer handle is managed by the engine internally;
            // it is freed when the engine is destroyed or the document is removed.
        }
    }

    private void ensureOpen() {
        if (closed) {
            throw new IllegalStateException("DocumentAnalyzer is already closed");
        }
    }
}
