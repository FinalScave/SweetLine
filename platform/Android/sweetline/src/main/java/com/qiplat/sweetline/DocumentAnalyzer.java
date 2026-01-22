package com.qiplat.sweetline;

import android.text.Spannable;

import com.qiplat.sweetline.util.NativeBufferPack;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * 高亮分析器
 */
public class DocumentAnalyzer {
    protected long nativeHandle;

    protected DocumentAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * 对整个文本进行高亮分析
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyze() {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readDocumentHighlight(buffer, stride);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param range patch的变更范围
     * @param newText patch的文本
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyzeIncremental(TextRange range, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = NativeBufferPack.packTextPosition(range.start);
        long endRange = NativeBufferPack.packTextPosition(range.end);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readDocumentHighlight(buffer, stride);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readDocumentHighlight(buffer, stride);
    }

    /**
     * 对整个文本进行分析，并将高亮结果转换为 {@link Spannable}
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeAsSpannable(SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, stride, styleFactory);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果，并将高亮结果转换为 {@link Spannable}
     * @param range patch的变更范围
     * @param newText patch的文本
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText, SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = NativeBufferPack.packTextPosition(range.start);
        long endRange = NativeBufferPack.packTextPosition(range.end);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, stride, styleFactory);
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果，并将高亮结果转换为 {@link Spannable}
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeIncrementalAsSpannable(int startIndex, int endIndex, String newText, SpannableStyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        int stride = nativeGetSpanBufferStride(nativeHandle);
        return NativeBufferPack.readSpannable(getDocument().getText(), buffer, stride, styleFactory);
    }

    /**
     * 获取托管文档对象
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
    @CriticalNative
    private static native long nativeGetDocument(long handle);
    @CriticalNative
    private static native int nativeGetSpanBufferStride(long handle);
}
