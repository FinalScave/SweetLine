package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

public class Document {
    protected long nativeHandle;

    protected Document(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    public Document(String uri, String content) {
        nativeHandle = nativeMakeDocument(uri, content);
    }

    public int getLineCount() {
        return nativeGetLineCount(nativeHandle);
    }

    public String getLine(int line) {
        return nativeGetLine(nativeHandle, line);
    }

    public String getText() {
        return nativeGetText(nativeHandle);
    }

    @FastNative
    private static native long nativeMakeDocument(String uri, String content);
    @CriticalNative
    private static native int nativeGetLineCount(long handle);
    @FastNative
    private static native String nativeGetLine(long handle, int line);
    @FastNative
    private static native String nativeGetText(long handle);
}
