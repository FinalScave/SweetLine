package com.qiplat.sweetline.demo;

import android.graphics.RectF;
import android.os.Bundle;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.util.SparseIntArray;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;

import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.markwon.CodeBackground;
import com.qiplat.sweetline.markwon.SweetLineGlobal;
import com.qiplat.sweetline.markwon.SweetLineHighlightPlugin;
import com.qiplat.sweetline.util.AssetUtils;

import io.noties.markwon.Markwon;

public class MarkwonActivity extends AppCompatActivity implements DocumentAnalyzer.StyleFactory {
    private AppCompatTextView mainTextView;
    private static SparseIntArray colorMap = new SparseIntArray();

    static {
        SweetLineGlobal.getEngineInstance().registerStyleName("keyword", 1);
        SweetLineGlobal.getEngineInstance().registerStyleName("string", 2);
        SweetLineGlobal.getEngineInstance().registerStyleName("number", 3);
        SweetLineGlobal.getEngineInstance().registerStyleName("comment", 4);
        SweetLineGlobal.getEngineInstance().registerStyleName("class", 5);
        SweetLineGlobal.getEngineInstance().registerStyleName("method", 6);
        SweetLineGlobal.getEngineInstance().registerStyleName("variable", 7);
        SweetLineGlobal.getEngineInstance().registerStyleName("punctuation", 8);
        SweetLineGlobal.getEngineInstance().registerStyleName("annotation", 9);
        colorMap.append(1, 0XFF569CD6);
        colorMap.append(2, 0XFFBD63C5);
        colorMap.append(3, 0XFFE4FAD5);
        colorMap.append(4, 0XFF60AE6F);
        colorMap.append(5, 0XFF4EC9B0);
        colorMap.append(6, 0XFF9CDCFE);
        colorMap.append(7, 0XFF9B9BC8);
        colorMap.append(8, 0XFFD69D85);
        colorMap.append(9, 0XFFFFFD9B);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_markwon);
        mainTextView = findViewById(R.id.main_text);
        preCompileSyntax();

        CodeBackground background = new CodeBackground(new RectF(100, 100, 100, 100), 0, null);
        SweetLineHighlightPlugin plugin = new SweetLineHighlightPlugin(background, this);
        Markwon markwon = Markwon.builder(this).usePlugin(plugin).build();

        try {
            String testMd = AssetUtils.readAsset(this, "Test.md");
            markwon.setMarkdown(mainTextView, testMd);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void preCompileSyntax() {
        try {
            String javaSyntaxJson = AssetUtils.readAsset(this, "java.json");
            String tiecodeSyntaxJson = AssetUtils.readAsset(this, "tiecode.json");
            SweetLineGlobal.getEngineInstance().compileSyntaxFromJson(javaSyntaxJson);
            SweetLineGlobal.getEngineInstance().compileSyntaxFromJson(tiecodeSyntaxJson);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public CharacterStyle createCharacterStyle(int styleId) {
        return new ForegroundColorSpan(colorMap.get(styleId));
    }

    @Override
    public CharacterStyle createCharacterStyle(InlineStyle inlineStyle) {
        return new ForegroundColorSpan(inlineStyle.foreground);
    }
}