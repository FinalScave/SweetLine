package com.qiplat.sweetline;

import dalvik.annotation.optimization.FastNative;

public class SyntaxRule {
    protected long nativeHandle;

    protected SyntaxRule(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    public String getName() {
        return nativeGetName(nativeHandle);
    }

    public String[] getFileExtensions() {
        return nativeGetFileExtensions(nativeHandle);
    }

    @FastNative
    private static native String nativeGetName(long handle);
    @FastNative
    private static native String[] nativeGetFileExtensions(long handle);
}
