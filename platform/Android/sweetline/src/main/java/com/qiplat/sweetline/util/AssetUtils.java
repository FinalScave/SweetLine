package com.qiplat.sweetline.util;

import android.content.Context;

import java.io.IOException;
import java.io.InputStream;

public class AssetUtils {
    /**
     * 读取assets中单个文件
     *
     * @param context  当前所处上下文环境
     * @param filename 文件名
     */
    public static String readAsset(Context context, String filename) throws IOException {
        InputStream inputstream = context.getAssets().open(filename);
        String text = StreamUtils.readStream(inputstream);
        return text;
    }
}
