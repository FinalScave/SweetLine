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

    public String getUri() {
        return nativeGetUri(nativeHandle);
    }

    public int totalChars() {
        return nativeTotalChars(nativeHandle);
    }

    public int getLineCharCount(int line) {
        return nativeGetLineCharCount(nativeHandle, line);
    }

    public int charIndexOfLine(int line) {
        return nativeCharIndexOfLine(nativeHandle, line);
    }

    public TextPosition charIndexToPosition(int index) {
        long value = nativeCharIndexToPosition(nativeHandle, index);
        int line = (int) (value >> 32);
        int column = (int) (value & 0XFFFFFFFFL);
        return new TextPosition(line, column, index);
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
    @FastNative
    private static native String nativeGetUri(long handle);
    @CriticalNative
    private static native int nativeTotalChars(long handle);
    @CriticalNative
    private static native int nativeGetLineCharCount(long handle, int line);
    @CriticalNative
    private static native int nativeCharIndexOfLine(long handle, int line);
    @CriticalNative
    private static native long nativeCharIndexToPosition(long handle, int index);
    @CriticalNative
    private static native int nativeGetLineCount(long handle);
    @FastNative
    private static native String nativeGetLine(long handle, int line);
    @FastNative
    private static native String nativeGetText(long handle);
}
