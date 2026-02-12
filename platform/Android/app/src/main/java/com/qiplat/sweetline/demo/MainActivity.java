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
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatButton;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.HighlightConfig;
import com.qiplat.sweetline.HighlightEngine;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.util.AssetUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends AppCompatActivity implements SpannableStyleFactory {
    private static final String TAG = "SampleHighlight";
    private Spinner fileSpinner;
    private MyEditText spanText;
    private CharSequence previousText;
    private DocumentAnalyzer analyzer;
    private boolean shouldAnalyzeChange = false;
    private static HighlightEngine inlineStyleEngine = new HighlightEngine(new HighlightConfig(true, true));
    private static HighlightEngine commonEngine = new HighlightEngine(new HighlightConfig(true, false));
    private static SparseIntArray colorMap = new SparseIntArray();
    private static final Map<String, String> EXT_SYNTAX_MAP = new HashMap<>();
    private static final List<String> INLINE_STYLE_FILES = new ArrayList<>();

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
        commonEngine.registerStyleName("preprocessor", 10);
        commonEngine.registerStyleName("macro", 11);
        commonEngine.registerStyleName("lifetime", 12);
        commonEngine.registerStyleName("selector", 13);
        commonEngine.registerStyleName("builtin", 14);
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

        EXT_SYNTAX_MAP.put(".t", "tiecode.json");
        EXT_SYNTAX_MAP.put(".c", "c.json");
        EXT_SYNTAX_MAP.put(".cpp", "cpp.json");
        EXT_SYNTAX_MAP.put(".cs", "csharp.json");
        EXT_SYNTAX_MAP.put(".dart", "dart.json");
        EXT_SYNTAX_MAP.put(".go", "go.json");
        EXT_SYNTAX_MAP.put(".groovy", "groovy.json");
        EXT_SYNTAX_MAP.put(".html", "html.json");
        EXT_SYNTAX_MAP.put(".java", "java.json");
        EXT_SYNTAX_MAP.put(".js", "javascript.json");
        EXT_SYNTAX_MAP.put(".json", "json-sweetline.json");
        EXT_SYNTAX_MAP.put(".kt", "kotlin.json");
        EXT_SYNTAX_MAP.put(".lua", "lua.json");
        EXT_SYNTAX_MAP.put(".py", "python.json");
        EXT_SYNTAX_MAP.put(".rs", "rust.json");
        EXT_SYNTAX_MAP.put(".sh", "shell.json");
        EXT_SYNTAX_MAP.put(".sql", "sql.json");
        EXT_SYNTAX_MAP.put(".swift", "swift.json");
        EXT_SYNTAX_MAP.put(".toml", "toml.json");
        EXT_SYNTAX_MAP.put(".ts", "typescript.json");
        EXT_SYNTAX_MAP.put(".xml", "xml.json");
        EXT_SYNTAX_MAP.put(".yaml", "yaml.json");
        EXT_SYNTAX_MAP.put(".md", "markdown.json");
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        fileSpinner = findViewById(R.id.file_spinner);
        spanText = findViewById(R.id.span_text);

        commonEngine.defineMacro("ANDROID");
        inlineStyleEngine.defineMacro("ANDROID");

        List<String> exampleFiles = listExampleFiles();
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                android.R.layout.simple_spinner_item, exampleFiles);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fileSpinner.setAdapter(adapter);

        fileSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String fileName = exampleFiles.get(position);
                shouldAnalyzeChange = false;
                highlightFile(fileName);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, 0, 0, "Markdown");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == 0) {
            startActivity(new Intent(this, MarkwonActivity.class));
        }
        return super.onOptionsItemSelected(item);
    }

    private List<String> listExampleFiles() {
        List<String> files = new ArrayList<>();
        try {
            String[] allAssets = getAssets().list("");
            if (allAssets != null) {
                Arrays.sort(allAssets);
                for (String name : allAssets) {
                    if (!name.startsWith("example") && !name.equals("json-sweetline.json")) {
                        continue;
                    }
                    String ext = getFileExtension(name);
                    if (EXT_SYNTAX_MAP.containsKey(ext)) {
                        files.add(name);
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "列出assets文件失败", e);
        }
        return files;
    }

    private void highlightFile(String testFileName) {
        String ext = getFileExtension(testFileName);
        String syntaxFileName = EXT_SYNTAX_MAP.get(ext);
        if (syntaxFileName == null || syntaxFileName.isEmpty()) {
            Log.w(TAG, "未找到 " + testFileName + " 对应的语法规则文件");
            return;
        }
        boolean inlineStyle = INLINE_STYLE_FILES.contains(ext);
        highlight(testFileName, syntaxFileName, inlineStyle);
    }

    private static String getFileExtension(String fileName) {
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex >= 0) {
            return fileName.substring(dotIndex);
        }
        return "";
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
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start + before,
                    "", this);
        } else if (before == 0 && count > 0) {
            // 插入文本
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "insert: index=" + start +
                    ", length=" + count + ", content='" + inserted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start,
                    inserted, this);
        } else if (before > 0 && count > 0) {
            // 替换文本
            String deleted = oldText.subSequence(start, start + before).toString();
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "replace: index=" + start +
                    ", deleted='" + deleted + "', inserted='" + inserted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start + before,
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
