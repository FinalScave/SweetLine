package com.qiplat.sweetline.util;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

/**
 * 输入输出流操作工具类
 * @author Scave
 */
public class StreamUtils {

    /**
     * 复制流
     * @param inputStream 输入流
     * @param outputStream 输出流
     */
    public static void copyStream(InputStream inputStream, OutputStream outputStream) throws IOException {
        byte[] bArr = new byte[1024];
        while (true) {
            int read = inputStream.read(bArr);
            if (read == -1) {
                inputStream.close();
                outputStream.close();
                return;
            }
            outputStream.write(bArr, 0, read);
        }
    }

    /**
     * 读取流
     * @param is 输入流
     * @return 文本
     */
    public static String readStream(InputStream is) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(is));
        boolean first = true;
        StringBuilder content = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            if (first) {
                first = false;
                content.append(line);
            } else {
                content.append(System.lineSeparator()).append(line);
            }
        }
        br.close();
        is.close();
        return content.toString();
    }

    /**
     * 读取流
     * @param is 输入流
     * @return 文本
     */
    public static byte[] readStreamBytes(InputStream is) throws IOException {
        byte[] bytes = new byte[is.available()];
        is.read(bytes);
        is.close();
        return bytes;
    }

    /**
     * 向一个输出流写入文本
     * @param out 输出流
     * @param text 欲写入的文本
     */
    public static void writeStreamString(OutputStream out, String text) throws IOException {
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(out));
        writer.write(text);
        writer.close();
        out.close();
    }

    /**
     * 向一个输出流写入字节集
     * @param out 输出流
     * @param bytes 欲写入的字节集
     */
    public static void writeStreamBytes(OutputStream out, byte[] bytes) throws IOException {
        writeStreamString(out, new String(bytes));
    }
}
