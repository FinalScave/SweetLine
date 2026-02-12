package com.qiplat.sweetline;

import android.text.Spannable;

import com.qiplat.sweetline.util.NativeBufferPack;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * 纯文本高亮分析器，不支持增量更新，适用于全量分析的场景
 */
public class TextAnalyzer {
    protected long nativeHandle;

    protected TextAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * 分析一段文本内容，并返回整段文本的高亮结果
     *
     * @param text 整段文本内容
     * @return 高亮结果
     */
    public DocumentHighlight analyzeText(String text) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeText(nativeHandle, text);
        return NativeBufferPack.readDocumentHighlight(buffer);
    }

    /**
     * 分析一段文本内容，并将高亮结果转换为 {@link Spannable}
     * @param text 整段文本内容
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
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
     * 分析单行文本
     *
     * @param text 单行文本内容
     * @param info 当前行元数据信息
     * @return 单行高亮分析结果
     */
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] packedTextLineInfo = NativeBufferPack.packTextLineInfo(info);
        int[] buffer = nativeAnalyzeLine(nativeHandle, text, packedTextLineInfo);
        return NativeBufferPack.readLineAnalyzeResult(buffer);
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
}