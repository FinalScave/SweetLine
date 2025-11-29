package com.qiplat.sweetline.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public final class FileUtils {
    /**
     * 读取文件内容
     *
     * @param path 文件路径
     */
    public static String readFile(String path) throws IOException {
        return readFile(new File(path));
    }

    /**
     * 读取文件
     *
     * @param file 文件
     */
    public static String readFile(File file) throws IOException {
        return StreamUtils.readStream(new FileInputStream(file));
    }

    /**
     * 读取文件字节
     *
     * @param file 文件
     */
    public static byte[] readFileBytes(File file) throws IOException {
        FileInputStream fis = new FileInputStream(file);
        byte[] bytes = new byte[fis.available()];
        fis.read(bytes);
        fis.close();
        return bytes;
    }
}
