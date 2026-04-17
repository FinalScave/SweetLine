package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * Highlight engine
 */
public class HighlightEngine {
    protected long nativeHandle;

    public HighlightEngine(HighlightConfig config) {
        nativeHandle = nativeMakeEngine(config.toNativeBits());
    }

    /**
     * Register a highlight style for name mapping
     * @param styleName Style name
     * @param styleId Style ID
     */
    public void registerStyleName(String styleName, int styleId) {
        if (nativeHandle == 0) {
            return;
        }
        nativeRegisterStyle(nativeHandle, styleName, styleId);
    }

    /**
     * Get the registered style name by style ID
     * @param styleId Style ID
     * @return Style name
     */
    public String getStyleName(int styleId) {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetStyleName(nativeHandle, styleId);
    }

    /**
     * Define a macro for controlling #ifdef conditional compilation in importSyntax
     * @param macroName Macro name
     */
    public void defineMacro(String macroName) {
        if (nativeHandle == 0) {
            return;
        }
        nativeDefineMacro(nativeHandle, macroName);
    }

    /**
     * Undefine a macro
     * @param macroName Macro name
     */
    public void undefineMacro(String macroName) {
        if (nativeHandle == 0) {
            return;
        }
        nativeUndefineMacro(nativeHandle, macroName);
    }

    /**
     * Compile syntax rule from JSON
     * @param syntaxJson JSON content of the syntax rule
     * @throws SyntaxCompileError on compilation error
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
     * Compile syntax rule
     * @param path Syntax rule definition file path (JSON)
     * @throws SyntaxCompileError on compilation error
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
     * Create a text highlight analyzer by syntax rule name (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
     * @param syntaxName Syntax rule name (e.g. java)
     */
    public TextAnalyzer createAnalyzerBySyntaxName(String syntaxName) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeCreateAnalyzerBySyntaxName(nativeHandle, syntaxName);
        return new TextAnalyzer(analyzerHandle);
    }

    /**
     * Create a text highlight analyzer by file name (no incremental analysis support, but supports single-line analysis with line state for custom incremental analysis)
     * @param fileName File name or basename used for syntax routing
     */
    public TextAnalyzer createAnalyzerByFileName(String fileName) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeCreateAnalyzerByFileName(nativeHandle, fileName);
        return new TextAnalyzer(analyzerHandle);
    }

    /**
     * Load a managed document and get a document highlight analyzer
     * @param document Managed document
     * @return Document highlight analyzer
     */
    public DocumentAnalyzer loadDocument(Document document) {
        if (nativeHandle == 0) {
            return null;
        }
        long analyzerHandle = nativeLoadDocument(nativeHandle, document.nativeHandle);
        return new DocumentAnalyzer(analyzerHandle);
    }

    /**
     * Remove a managed document
     * @param uri Managed document URI
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
    private static native void nativeDefineMacro(long handle, String macroName);
    @FastNative
    private static native void nativeUndefineMacro(long handle, String macroName);
    @FastNative
    private static native long nativeCompileSyntaxFromJson(long handle, String json) throws SyntaxCompileError;
    @FastNative
    private static native long nativeCompileSyntaxFromFile(long handle, String path) throws SyntaxCompileError;
    @FastNative
    private static native long nativeCreateAnalyzerBySyntaxName(long handle, String syntaxName);
    @FastNative
    private static native long nativeCreateAnalyzerByFileName(long handle, String fileName);
    @CriticalNative
    private static native long nativeLoadDocument(long handle, long documentHandle);
    @FastNative
    private static native void nativeRemoveDocument(long handle, String uri);

    static {
        System.loadLibrary("sweetline");
    }
}
