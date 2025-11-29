package com.qiplat.sweetline;

import android.text.Spannable;

import com.qiplat.sweetline.util.NativeBufferPack;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * Highlight analyzer
 */
public class DocumentAnalyzer {
    protected long nativeHandle;

    protected DocumentAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * Perform full highlight analysis on the text
     * @return Highlight result for the entire text
     */
    public DocumentHighlight analyze() {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Incrementally re-analyze based on patch content, returningHighlight result for the entire text
     * @param range Change range of the patch
     * @param newText Patched text
     * @return Highlight result for the entire text
     */
    public DocumentHighlight analyzeIncremental(TextRange range, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = NativeBufferPack.packTextPosition(range.start);
        long endRange = NativeBufferPack.packTextPosition(range.end);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Incrementally re-analyze based on patch content, returningHighlight result for the entire text
     * @param startIndex Start index of the patch change
     * @param endIndex End index of the patch change
     * @param newText Patched text
     * @return Highlight result for the entire text
     */
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Incrementally re-analyze and return highlight slice for specified visible line range
     * @param range Change range of the patch (line/column)
     * @param newText Patched text
     * @param visibleRange Visible line range (startLine + lineCount)
     * @return Highlight slice for the specified line range
     */
    public DocumentHighlightSlice analyzeIncrementalInLineRange(TextRange range, String newText, LineRange visibleRange) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = NativeBufferPack.packTextPosition(range.start);
        long endRange = NativeBufferPack.packTextPosition(range.end);
        int[] buffer = nativeAnalyzeChangesInLineRange(nativeHandle, startRange, endRange, newText,
                visibleRange.startLine, visibleRange.lineCount);
        return NativeBufferPack.readDocumentHighlightSlice(buffer);
    }

    /**
     * Perform full text analysis and convert highlight result to {@link Spannable}
     * @param styleFactory Span style factory, creates Span style by styleId
     * @return {@link Spannable}
     */
    public Spannable analyzeAsSpannable(SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, styleFactory);
    }

    /**
     * Incrementally re-analyze based on patch content, returningHighlight result for the entire text，and convert highlight result to {@link Spannable}
     * @param range Change range of the patch
     * @param newText Patched text
     * @param styleFactory Span style factory, creates Span style by styleId
     * @return {@link Spannable}
     */
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText, SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = NativeBufferPack.packTextPosition(range.start);
        long endRange = NativeBufferPack.packTextPosition(range.end);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, styleFactory);
    }

    /**
     * Incrementally re-analyze based on patch content, returningHighlight result for the entire text，and convert highlight result to {@link Spannable}
     * @param startIndex Start index of the patch change
     * @param endIndex End index of the patch change
     * @param newText Patched text
     * @param styleFactory Span style factory, creates Span style by styleId
     * @return {@link Spannable}
     */
    public Spannable analyzeIncrementalAsSpannable(int startIndex, int endIndex, String newText, SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, styleFactory);
    }

    /**
     * Perform indent guide analysis on the managed document (requires prior call to analyze or analyzeIncremental)
     * @return Indent guide analysis result
     */
    public IndentGuideResult analyzeIndentGuides() {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeIndentGuides(nativeHandle);
        return NativeBufferPack.readIndentGuideResult(buffer);
    }

    /**
     * Get the managed document
     * @return {@link Document}
     */
    public Document getDocument() {
        if (nativeHandle == 0) {
            return null;
        }
        return new Document(nativeGetDocument(nativeHandle));
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (nativeHandle != 0) {
            nativeFinalizeAnalyzer(nativeHandle);
        }
        nativeHandle = 0;
    }

    @CriticalNative
    private static native void nativeFinalizeAnalyzer(long handle);

    @FastNative
    private static native int[] nativeAnalyze(long handle);

    @FastNative
    private static native int[] nativeAnalyzeChanges(long handle, long startPosition, long endPosition, String newText);

    @FastNative
    private static native int[] nativeAnalyzeChanges2(long handle, int startIndex, int endIndex, String newText);

    @FastNative
    private static native int[] nativeAnalyzeChangesInLineRange(long handle, long startPosition, long endPosition,
                                                                String newText, int visibleStartLine, int visibleLineCount);

    @FastNative
    private static native int[] nativeAnalyzeIndentGuides(long handle);

    @CriticalNative
    private static native long nativeGetDocument(long handle);
}
