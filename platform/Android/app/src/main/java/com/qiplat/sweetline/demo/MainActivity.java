package com.qiplat.sweetline.demo;

import android.content.Intent;
import android.os.Bundle;
import android.text.Editable;
import android.text.Spannable;
import android.text.TextWatcher;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.util.SparseIntArray;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatButton;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.HighlightConfig;
import com.qiplat.sweetline.HighlightEngine;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.util.AssetUtils;

public class MainActivity extends AppCompatActivity implements DocumentAnalyzer.StyleFactory {
    private static final String TAG = "SampleHighlight";
    private AppCompatButton testJavaButton;
    private AppCompatButton testTiecodeButton;
    private AppCompatButton testJsonButton;
    private AppCompatButton gotoMarkwonBtn;
    private MyEditText spanText;
    private CharSequence previousText;
    private DocumentAnalyzer analyzer;
    private boolean shouldAnalyzeChange = false;
    private static HighlightEngine inlineStyleEngine = new HighlightEngine(new HighlightConfig(true, true));
    private static HighlightEngine commonEngine = new HighlightEngine(new HighlightConfig(true, false));
    private static SparseIntArray colorMap = new SparseIntArray();

    static {
        commonEngine.registerStyleName("keyword", 1);
        commonEngine.registerStyleName("string", 2);
        commonEngine.registerStyleName("number", 3);
        commonEngine.registerStyleName("comment", 4);
        commonEngine.registerStyleName("class", 5);
        commonEngine.registerStyleName("method", 6);
        commonEngine.registerStyleName("variable", 7);
        commonEngine.registerStyleName("punctuation", 8);
        commonEngine.registerStyleName("annotation", 9);
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
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        testJavaButton = findViewById(R.id.test_java_btn);
        testTiecodeButton = findViewById(R.id.test_tiecode_btn);
        testJsonButton = findViewById(R.id.test_json_btn);
        gotoMarkwonBtn = findViewById(R.id.test_markwon_btn);
        spanText = findViewById(R.id.span_text);
        testJavaButton.setOnClickListener(v -> {
            shouldAnalyzeChange = false;
            highlight("Main.java", "java.json", false);
        });
        testTiecodeButton.setOnClickListener(v -> {
            shouldAnalyzeChange = false;
            highlight("结绳.t", "tiecode-inlineStyle.json", true);
        });
        testJsonButton.setOnClickListener(v -> {
            shouldAnalyzeChange = false;
            highlight("java.json", "json-sweetline.json", false);
        });
        gotoMarkwonBtn.setOnClickListener(v -> {
            startActivity(new Intent(this, MarkwonActivity.class));
        });
        spanText.setOnSelectionChangeListener((startIndex, endIndex) -> {
            Log.i(TAG, String.format("start: %d, end: %d", startIndex, endIndex));
        });
        spanText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                previousText = s.toString();
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if (!shouldAnalyzeChange) {
                    shouldAnalyzeChange = true;
                    return;
                }
                analyzeChanges(previousText, s.toString(), start, before, count);
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });
    }

    private void highlight(String testFileName, String syntaxFileName, boolean inlineStyle) {
        try {
            String testCode = AssetUtils.readAsset(this, testFileName);
            String json = AssetUtils.readAsset(this, syntaxFileName);

            long start = System.nanoTime();
            if (inlineStyle) {
                inlineStyleEngine.compileSyntaxFromJson(json);
            } else {
                commonEngine.compileSyntaxFromJson(json);
            }
            long end = System.nanoTime();
            Log.i(TAG, String.format("====compileSyntaxFromJson: %dus", (end - start) / 1000));

            Document document = new Document(testFileName, testCode);

            start = System.nanoTime();
            if (inlineStyle) {
                analyzer = inlineStyleEngine.loadDocument(document);
            } else {
                analyzer = commonEngine.loadDocument(document);
            }
            end = System.nanoTime();
            Log.i(TAG, String.format("====loadDocument: %dus", (end - start) / 1000));

            start = System.nanoTime();
            Spannable highlightedText = analyzer.analyzeAsSpannable(this);
            end = System.nanoTime();
            Log.i(TAG, String.format("====analyze: %dus", (end - start) / 1000));

            spanText.setText(highlightedText);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void analyzeChanges(CharSequence oldText, CharSequence newText,
                                int start, int before, int count) {
        if (analyzer == null) {
            return;
        }
        long startTime = System.nanoTime();
        Spannable newHighlightedText = null;
        if (before > 0 && count == 0) {
            // 删除文本
            String deleted = oldText.subSequence(start, start + before).toString();
            Log.d(TAG, "delete: index=" + start +
                    ", length=" + before + ", content='" + deleted + "'");
            newHighlightedText = analyzer.analyzeChangesAsSpannable(start, start + before,
                    "", this);
        } else if (before == 0 && count > 0) {
            // 插入文本
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "insert: index=" + start +
                    ", length=" + count + ", content='" + inserted + "'");
            newHighlightedText = analyzer.analyzeChangesAsSpannable(start, start,
                    inserted, this);
        } else if (before > 0 && count > 0) {
            // 替换文本
            String deleted = oldText.subSequence(start, start + before).toString();
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "replace: index=" + start +
                    ", deleted='" + deleted + "', inserted='" + inserted + "'");
            newHighlightedText = analyzer.analyzeChangesAsSpannable(start, start + before,
                    inserted, this);
        }
        long endTime = System.nanoTime();
        Log.i(TAG, String.format("====analyzeChanges: %dus", (endTime - startTime) / 1000));
        shouldAnalyzeChange = false;
        int selectionStart = spanText.getSelectionStart();
        spanText.setText(newHighlightedText);
        spanText.setSelection(selectionStart);
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
