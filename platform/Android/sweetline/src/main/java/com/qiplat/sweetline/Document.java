package com.qiplat.sweetline;

import com.qiplat.sweetline.util.NativeBufferPack;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * Managed document with incremental update support
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
     * Get the URI of the managed document
     */
    public String getUri() {
        if (nativeHandle == 0) {
            return null;
        }
        return nativeGetUri(nativeHandle);
    }

    /**
     * Total character count of the document
     * @return Total character count
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
        return NativeBufferPack.unpackTextPosition(value);
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
