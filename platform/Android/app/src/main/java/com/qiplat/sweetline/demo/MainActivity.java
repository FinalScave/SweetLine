package com.qiplat.sweetline.demo;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.text.Editable;
import android.text.Spannable;
import android.text.TextWatcher;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

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
    private View rootView;
    private Spinner fileSpinner;
    private MyEditText spanText;
    private TextView themeLabel;
    private CharSequence previousText;
    private DocumentAnalyzer analyzer;
    private boolean shouldAnalyzeChange = false;

    private static HighlightEngine inlineStyleEngine = new HighlightEngine(new HighlightConfig(true, true));
    private static HighlightEngine commonEngine = new HighlightEngine(new HighlightConfig(true, false));
    private static final Map<String, String> EXT_SYNTAX_MAP = new HashMap<>();
    private static final List<String> INLINE_STYLE_FILES = new ArrayList<>();

    private List<HighlightTheme> themes;
    private int currentThemeIndex = 0;
    private HighlightTheme currentTheme;

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
        EXT_SYNTAX_MAP.put(".php", "php.json");
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
        EXT_SYNTAX_MAP.put(".wenyan", "wenyan.json");
        EXT_SYNTAX_MAP.put(".myu", "iapp.json");
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        rootView = findViewById(R.id.main);
        fileSpinner = findViewById(R.id.file_spinner);
        spanText = findViewById(R.id.span_text);
        themeLabel = findViewById(R.id.theme_label);

        commonEngine.defineMacro("ANDROID");
        inlineStyleEngine.defineMacro("ANDROID");

        themes = HighlightTheme.builtinThemes();
        currentTheme = themes.get(currentThemeIndex);
        applyThemeColors();

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
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.action_theme) {
            showThemeDialog();
            return true;
        } else if (id == R.id.action_markdown) {
            startActivity(new Intent(this, MarkwonActivity.class));
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showThemeDialog() {
        String[] names = new String[themes.size()];
        for (int i = 0; i < themes.size(); i++) {
            names[i] = themes.get(i).name;
        }

        new AlertDialog.Builder(this)
                .setTitle("选择高亮主题")
                .setSingleChoiceItems(names, currentThemeIndex, (dialog, which) -> {
                    currentThemeIndex = which;
                    currentTheme = themes.get(which);
                    applyThemeColors();
                    reHighlightCurrentFile();
                    dialog.dismiss();
                })
                .setNegativeButton("取消", null)
                .show();
    }

    private void applyThemeColors() {
        rootView.setBackgroundColor(currentTheme.backgroundColor);
        spanText.setTextColor(currentTheme.textColor);

        if (getSupportActionBar() != null) {
            getSupportActionBar().setSubtitle(currentTheme.name);
        }

        themeLabel.setText(currentTheme.name);
        themeLabel.setTextColor(currentTheme.textColor);

        int statusBarColor = darkenColor(currentTheme.backgroundColor, 0.7f);
        getWindow().setStatusBarColor(statusBarColor);

        if (getSupportActionBar() != null) {
            getSupportActionBar().setBackgroundDrawable(
                    new ColorDrawable(statusBarColor));
        }
    }

    private void reHighlightCurrentFile() {
        int pos = fileSpinner.getSelectedItemPosition();
        if (pos >= 0) {
            Object item = fileSpinner.getItemAtPosition(pos);
            if (item != null) {
                shouldAnalyzeChange = false;
                highlightFile(item.toString());
            }
        }
    }

    private static int darkenColor(int color, float factor) {
        int a = Color.alpha(color);
        int r = Math.round(Color.red(color) * factor);
        int g = Math.round(Color.green(color) * factor);
        int b = Math.round(Color.blue(color) * factor);
        return Color.argb(a, Math.min(r, 255), Math.min(g, 255), Math.min(b, 255));
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
            String deleted = oldText.subSequence(start, start + before).toString();
            Log.d(TAG, "delete: index=" + start +
                    ", length=" + before + ", content='" + deleted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start + before,
                    "", this);
        } else if (before == 0 && count > 0) {
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "insert: index=" + start +
                    ", length=" + count + ", content='" + inserted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start,
                    inserted, this);
        } else if (before > 0 && count > 0) {
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
        return new ForegroundColorSpan(currentTheme.colorMap.get(styleId));
    }

    @Override
    public CharacterStyle createCharacterStyle(InlineStyle inlineStyle) {
        return new ForegroundColorSpan(inlineStyle.foreground);
    }
}
