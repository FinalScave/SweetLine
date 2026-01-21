package com.qiplat.sweetline;

import dalvik.annotation.optimization.FastNative;

/**
 * 语法规则
 */
public class SyntaxRule {
    protected long nativeHandle;

    protected SyntaxRule(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * 获取语法规则的名称
     */
    public String getName() {
        return nativeGetName(nativeHandle);
    }

    /**
     * 获取语法规则支持的文件扩展名
     * @return 扩展名数组
     */
    public String[] getFileExtensions() {
        return nativeGetFileExtensions(nativeHandle);
    }

    @FastNative
    private static native String nativeGetName(long handle);
    @FastNative
    private static native String[] nativeGetFileExtensions(long handle);
}
