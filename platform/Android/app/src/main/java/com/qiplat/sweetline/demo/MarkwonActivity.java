package com.qiplat.sweetline.demo;

import android.graphics.RectF;
import android.os.Bundle;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.util.SparseIntArray;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;

import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.markwon.CodeBackground;
import com.qiplat.sweetline.markwon.SweetLineGlobal;
import com.qiplat.sweetline.markwon.SweetLineHighlightPlugin;
import com.qiplat.sweetline.util.AssetUtils;

import io.noties.markwon.Markwon;

public class MarkwonActivity extends AppCompatActivity implements SpannableStyleFactory {
    private static final String SYNTAX_ASSET_DIR = "syntaxes";
    private static final String EXAMPLE_ASSET_DIR = "examples";
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
        SweetLineGlobal.getEngineInstance().registerStyleName("preprocessor", 10);
        SweetLineGlobal.getEngineInstance().registerStyleName("macro", 11);
        SweetLineGlobal.getEngineInstance().registerStyleName("lifetime", 12);
        SweetLineGlobal.getEngineInstance().registerStyleName("selector", 13);
        SweetLineGlobal.getEngineInstance().registerStyleName("builtin", 14);
        SweetLineGlobal.getEngineInstance().registerStyleName("url", 15);
        SweetLineGlobal.getEngineInstance().registerStyleName("property", 16);
        colorMap.append(1, 0XFF569CD6);
        colorMap.append(2, 0XFFBD63C5);
        colorMap.append(3, 0XFFE4FAD5);
        colorMap.append(4, 0XFF60AE6F);
        colorMap.append(5, 0XFF4EC9B0);
        colorMap.append(6, 0XFF9CDCFE);
        colorMap.append(7, 0XFF9B9BC8);
        colorMap.append(8, 0XFFD69D85);
        colorMap.append(9, 0XFFFFFD9B);
        colorMap.append(10, 0XFF569CD6);
        colorMap.append(11, 0XFF9B9BC8);
        colorMap.append(12, 0XFF4EC9B0);
        colorMap.append(13, 0XFF4EC9B0);
        colorMap.append(14, 0XFF569CD6);
        colorMap.append(15, 0XFF4FC1FF);
        colorMap.append(16, 0XFF9CDCFE);
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
            String testMd = AssetUtils.readAsset(this, EXAMPLE_ASSET_DIR + "/example.md");
            markwon.setMarkdown(mainTextView, testMd);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void preCompileSyntax() {
        try {
            String[] syntaxFiles = {
                    "c.json", "cpp.json", "csharp.json", "dart.json", "go.json", "groovy.json",
                    "javascript.json", "html.json", "java.json", "json-sweetline.json", "jsonc.json", "json5.json",
                    "kotlin.json", "lua.json", "python.json", "rust.json", "scala.json", "shell.json", "sql.json",
                    "swift.json", "toml.json", "typescript.json", "xml.json", "yaml.json", "tiecode.json",
                    "markdown.json", "css.json", "scss.json", "less.json", "cmake.json", "dockerfile.json",
                    "makefile.json", "properties.json", "env.json"
            };
            for (String syntaxFile : syntaxFiles) {
                String syntaxJson = AssetUtils.readAsset(this, SYNTAX_ASSET_DIR + "/" + syntaxFile);
                SweetLineGlobal.getEngineInstance().compileSyntaxFromJson(syntaxJson);
            }
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
