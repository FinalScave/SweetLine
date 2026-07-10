package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * Syntax rule
 */
public class SyntaxRule implements AutoCloseable {
    protected volatile long nativeHandle;

    protected SyntaxRule(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * Get the name of the syntax rule
     */
    public String getName() {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetName(nativeHandle);
    }

    /**
     * Get the exact file names supported by the syntax rule.
     * @return Array of file names
     */
    public String[] getFileNames() {
        if (nativeHandle == 0) {
            return new String[0];
        }
        return nativeGetFileNames(nativeHandle);
    }

    /**
     * Get the file suffixes supported by the syntax rule.
     * @return Array of file suffixes
     */
    public String[] getFileSuffixes() {
        if (nativeHandle == 0) {
            return new String[0];
        }
        return nativeGetFileSuffixes(nativeHandle);
    }

    @Override
    public synchronized void close() {
        long handle = nativeHandle;
        if (handle != 0) {
            nativeHandle = 0;
            nativeFinalizeRule(handle);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    @CriticalNative
    private static native void nativeFinalizeRule(long handle);
    @FastNative
    private static native String nativeGetName(long handle);
    @FastNative
    private static native String[] nativeGetFileNames(long handle);
    @FastNative
    private static native String[] nativeGetFileSuffixes(long handle);
}
