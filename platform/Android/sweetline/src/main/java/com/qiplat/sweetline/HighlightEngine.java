package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

public class HighlightEngine {
    protected long nativeHandle;

    public HighlightEngine(HighlightConfig config) {
        nativeHandle = nativeMakeEngine(config.showIndex);
    }

    public void registerStyleName(String styleName, int styleId) {
        nativeRegisterStyle(nativeHandle, styleName, styleId);
    }

    public String getStyleName(int styleId) {
        return nativeGetStyleName(nativeHandle, styleId);
    }

    public SyntaxRule compileSyntaxFromJson(String syntaxJson) throws SyntaxCompileError {
        long handle = nativeCompileSyntaxFromJson(nativeHandle, syntaxJson);
        if (handle == 0) {
            return null;
        } else {
            return new SyntaxRule(handle);
        }
    }

    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError {
        long handle = nativeCompileSyntaxFromFile(nativeHandle, path);
        if (handle == 0) {
            return null;
        } else {
            return new SyntaxRule(handle);
        }
    }

    public DocumentAnalyzer loadDocument(Document document) {
        long analyzerHandle = nativeLoadDocument(nativeHandle, document.nativeHandle);
        return new DocumentAnalyzer(analyzerHandle);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        nativeFinalizeEngine(nativeHandle);
        nativeHandle = 0;
    }

    @CriticalNative
    private static native long nativeMakeEngine(boolean show_index);
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
    @CriticalNative
    private static native long nativeLoadDocument(long handle, long documentHandle);

    static {
        System.loadLibrary("sweetline");
    }
}
