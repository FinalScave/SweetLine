package com.qiplat.sweetline;

import dalvik.annotation.optimization.FastNative;

/**
 * Syntax rule
 */
public class SyntaxRule {
    protected long nativeHandle;

    protected SyntaxRule(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * Get the name of the syntax rule
     */
    public String getName() {
        return nativeGetName(nativeHandle);
    }

    /**
     * Get the file extensions supported by the syntax rule
     * @return Array of file extensions
     */
    public String[] getFileExtensions() {
        return nativeGetFileExtensions(nativeHandle);
    }

    @FastNative
    private static native String nativeGetName(long handle);
    @FastNative
    private static native String[] nativeGetFileExtensions(long handle);
}
