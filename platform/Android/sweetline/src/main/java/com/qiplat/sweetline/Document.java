package com.qiplat.sweetline;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * 支持增量更新的托管文档
 */
public class Document {
    protected long nativeHandle;

    protected Document(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    public Document(String uri, String content) {
        nativeHandle = nativeMakeDocument(uri, content);
    }

    /**
     * 获取托管文档的Uri
     */
    public String getUri() {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetUri(nativeHandle);
    }

    /**
     * 文档字符总数
     * @return 字符总数
     */
    public int totalChars() {
        if (nativeHandle == 0) {
            return -1;
        }
        return nativeTotalChars(nativeHandle);
    }

    public int getLineCharCount(int line) {
        if (nativeHandle == 0) {
            return -1;
        }
        return nativeGetLineCharCount(nativeHandle, line);
    }

    public int charIndexOfLine(int line) {
        if (nativeHandle == 0) {
            return -1;
        }
        return nativeCharIndexOfLine(nativeHandle, line);
    }

    public TextPosition charIndexToPosition(int index) {
        if (nativeHandle == 0) {
            return null;
        }
        long value = nativeCharIndexToPosition(nativeHandle, index);
        int line = (int) (value >> 32);
        int column = (int) (value & 0XFFFFFFFFL);
        return new TextPosition(line, column, index);
    }

    public int getLineCount() {
        if (nativeHandle == 0) {
            return -1;
        }
        return nativeGetLineCount(nativeHandle);
    }

    public String getLine(int line) {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetLine(nativeHandle, line);
    }

    public String getText() {
        if (nativeHandle == 0) {
            return null;
        }
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
