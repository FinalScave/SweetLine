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
     * Get the exact file names supported by the syntax rule.
     * @return Array of file names
     */
    public String[] getFileNames() {
        return nativeGetFileNames(nativeHandle);
    }

    /**
     * Get the file suffixes supported by the syntax rule.
     * @return Array of file suffixes
     */
    public String[] getFileSuffixes() {
        return nativeGetFileSuffixes(nativeHandle);
    }

    @FastNative
    private static native String nativeGetName(long handle);
    @FastNative
    private static native String[] nativeGetFileNames(long handle);
    @FastNative
    private static native String[] nativeGetFileSuffixes(long handle);
}
