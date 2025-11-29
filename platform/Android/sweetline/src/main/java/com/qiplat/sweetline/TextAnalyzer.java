package com.qiplat.sweetline;

import android.text.Spannable;

import com.qiplat.sweetline.util.NativeBufferPack;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * Plain text highlight analyzer, no incremental update support, suitable for full analysis scenarios
 */
public class TextAnalyzer {
    protected long nativeHandle;

    protected TextAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * Analyze a text and return the highlight result for the entire text
     *
     * @param text Full text content
     * @return Highlight result
     */
    public DocumentHighlight analyzeText(String text) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeText(nativeHandle, text);
        return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * Analyze a text and convert highlight result to {@link Spannable}
     * @param text Full text content
     * @param styleFactory Span style factory, creates Span style by styleId
     * @return {@link Spannable}
     */
    public Spannable analyzeTextAsSpannable(String text, SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeText(nativeHandle, text);
        return NativeBufferPack.readSpannable(text, buffer, styleFactory);
    }

    /**
     * Analyze a single line of text
     *
     * @param text Single line text content
     * @param info Metadata for the current line
     * @return Single line analysis result
     */
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] packedTextLineInfo = NativeBufferPack.packTextLineInfo(info);
        int[] buffer = nativeAnalyzeLine(nativeHandle, text, packedTextLineInfo);
        return NativeBufferPack.readLineAnalyzeResult(buffer);
    }

    /**
     * Perform indent guide analysis on a text (internally performs highlight analysis first)
     *
     * @param text Full text content
     * @return Indent guide analysis result
     */
    public IndentGuideResult analyzeIndentGuides(String text) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeIndentGuides(nativeHandle, text);
        return NativeBufferPack.readIndentGuideResult(buffer);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (nativeHandle != 0) {
            nativeFinalize(nativeHandle);
        }
        nativeHandle = 0;
    }

    @FastNative
    private static native void nativeFinalize(long handle);

    @FastNative
    private static native int[] nativeAnalyzeText(long handle, String text);

    @FastNative
    private static native int[] nativeAnalyzeLine(long handle, String text, int[] info);

    @FastNative
    private static native int[] nativeAnalyzeIndentGuides(long handle, String text);
}