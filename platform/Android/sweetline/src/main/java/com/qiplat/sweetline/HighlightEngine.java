package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * 高亮引擎
 */
public class HighlightEngine {
    protected long nativeHandle;

    public HighlightEngine(HighlightConfig config) {
        nativeHandle = nativeMakeEngine(config.toNativeBits());
    }

    /**
     * 注册一个高亮样式，用于名称映射
     * @param styleName 样式名称
     * @param styleId 样式id
     */
    public void registerStyleName(String styleName, int styleId) {
        if (nativeHandle == 0) {
            return;
        }
        nativeRegisterStyle(nativeHandle, styleName, styleId);
    }

    /**
     * 通过样式id获取注册的样式名称
     * @param styleId 样式id
     * @return 样式名称
     */
    public String getStyleName(int styleId) {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetStyleName(nativeHandle, styleId);
    }

    /**
     * 通过json编译语法规则
     * @param syntaxJson 语法规则文件的json
     * @throws SyntaxCompileError 编译错误时会抛出 SyntaxRuleParseError
     */
    public SyntaxRule compileSyntaxFromJson(String syntaxJson) throws SyntaxCompileError {
        if (nativeHandle == 0) {
            return null;
        }
        long handle = nativeCompileSyntaxFromJson(nativeHandle, syntaxJson);
        if (handle == 0) {
            return null;
        } else {
            return new SyntaxRule(handle);
        }
    }

    /**
     * 编译语法规则
     * @param path 语法规则定义文件路径(json)
     * @throws SyntaxCompileError 编译错误时会抛出 SyntaxRuleParseError
     */
    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError {
        if (nativeHandle == 0) {
            return null;
        }
        long handle = nativeCompileSyntaxFromFile(nativeHandle, path);
        if (handle == 0) {
            return null;
        } else {
            return new SyntaxRule(handle);
        }
    }

    /**
     * 根据语法规则名称创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
     * @param syntaxName 语法规则名称(如 java)
     */
    public TextAnalyzer createAnalyzerByName(String syntaxName) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeCreateAnalyzerByName(nativeHandle, syntaxName);
        return new TextAnalyzer(analyzerHandle);
    }

    /**
     * 根据文件后缀名创建一个文本高亮分析器(不支持增量分析,但可以分析单行并获得行状态,可以在上层自行实现增量分析)
     * @param extension 文件后缀名(如 .t)
     */
    public TextAnalyzer createAnalyzerByExtension(String extension) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeCreateAnalyzerByExtension(nativeHandle, extension);
        return new TextAnalyzer(analyzerHandle);
    }

    /**
     * 加载托管文档对象获得文档高亮分析器
     * @param document 托管文档对象
     * @return 文档高亮分析器
     */
    public DocumentAnalyzer loadDocument(Document document) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeLoadDocument(nativeHandle, document.nativeHandle);
        return new DocumentAnalyzer(analyzerHandle);
    }

    /**
     * 移除托管文档
     * @param uri 托管文档Uri
     */
    public void removeDocument(String uri) {
        if (nativeHandle == 0) {
            return;
        }
        nativeRemoveDocument(nativeHandle, uri);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (nativeHandle != 0) {
            nativeFinalizeEngine(nativeHandle);
        }
        nativeHandle = 0;
    }

    @CriticalNative
    private static native long nativeMakeEngine(int configBits);
    @CriticalNative
    private static native void nativeFinalizeEngine(long handle);
    @FastNative
    private static native void nativeRegisterStyle(long handle, String styleName, int styleId);
    @FastNative
    private static native String nativeGetStyleName(long handle, int styleId);
    @FastNative
    private static native long nativeCompileSyntaxFromJson(long handle, String json) throws SyntaxCompileError;
    @FastNative
    private static native long nativeCompileSyntaxFromFile(long handle, String path) throws SyntaxCompileError;
    @FastNative
    private static native long nativeCreateAnalyzerByName(long handle, String syntaxName);
    @FastNative
    private static native long nativeCreateAnalyzerByExtension(long handle, String extension);
    @CriticalNative
    private static native long nativeLoadDocument(long handle, long documentHandle);
    @FastNative
    private static native void nativeRemoveDocument(long handle, String uri);

    static {
        System.loadLibrary("sweetline");
    }
}
