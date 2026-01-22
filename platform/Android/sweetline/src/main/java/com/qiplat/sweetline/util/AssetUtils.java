package com.qiplat.sweetline.util;

import android.content.Context;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class AssetUtils {
    /**
     * 读取assets中单个文件
     *
     * @param context  当前所处上下文环境
     * @param filename 文件名
     */
    public static String readAsset(Context context, String filename) throws IOException {
        InputStream inputstream = context.getAssets().open(filename);
        String text = readStream(inputstream);
        return text;
    }

    private static String readStream(InputStream is) throws IOException {
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
}
