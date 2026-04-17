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
    private static final Map<String, String> EXACT_SYNTAX_MAP = new HashMap<>();
    private static final Map<String, String> SUFFIX_SYNTAX_MAP = new HashMap<>();
    private static final Map<String, String> ROUTED_DOCUMENT_NAMES = new HashMap<>();
    private static final List<String> SORTED_SUFFIXES = new ArrayList<>();
    private static final List<String> EXACT_SAMPLE_FILES = Arrays.asList(
            ".gitignore",
            "CMakeLists.txt",
            "Containerfile",
            "Dockerfile",
            "GNUmakefile",
            "Makefile",
            "makefile"
    );
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
        commonEngine.registerStyleName("url", 15);
        commonEngine.registerStyleName("property", 16);

        registerExactRoute(".gitignore", "gitignore.json", "example.gitignore");
        registerExactRoute("CMakeLists.txt", "cmake.json", "example.cmake");
        registerExactRoute("Containerfile", "dockerfile.json", "example.dockerfile");
        registerExactRoute("Dockerfile", "dockerfile.json", "example.dockerfile");
        registerExactRoute("GNUmakefile", "makefile.json", "example.mk");
        registerExactRoute("Makefile", "makefile.json", "example.mk");
        registerExactRoute("makefile", "makefile.json", "example.mk");

        registerSuffixRoute(".t", "tiecode.json");
        registerSuffixRoute(".c", "c.json");
        registerSuffixRoute(".cpp", "cpp.json");
        registerSuffixRoute(".cs", "csharp.json");
        registerSuffixRoute(".dart", "dart.json");
        registerSuffixRoute(".go", "go.json");
        registerSuffixRoute(".groovy", "groovy.json");
        registerSuffixRoute(".html", "html.json");
        registerSuffixRoute(".java", "java.json");
        registerSuffixRoute(".js", "javascript.json");
        registerSuffixRoute(".json", "json-sweetline.json");
        registerSuffixRoute(".jsonc", "jsonc.json");
        registerSuffixRoute(".json5", "json5.json");
        registerSuffixRoute(".kt", "kotlin.json");
        registerSuffixRoute(".lua", "lua.json");
        registerSuffixRoute(".m", "objc.json");
        registerSuffixRoute(".php", "php.json");
        registerSuffixRoute(".ps1", "powershell.json");
        registerSuffixRoute(".py", "python.json");
        registerSuffixRoute(".rs", "rust.json");
        registerSuffixRoute(".scala", "scala.json");
        registerSuffixRoute(".sh", "shell.json");
        registerSuffixRoute(".sql", "sql.json");
        registerSuffixRoute(".swift", "swift.json");
        registerSuffixRoute(".toml", "toml.json");
        registerSuffixRoute(".ts", "typescript.json");
        registerSuffixRoute(".vb", "vb.json");
        registerSuffixRoute(".xml", "xml.json");
        registerSuffixRoute(".yaml", "yaml.json");
        registerSuffixRoute(".md", "markdown.json");
        registerSuffixRoute(".wenyan", "wenyan.json");
        registerSuffixRoute(".myu", "iapp.json");
        registerSuffixRoute(".css", "css.json");
        registerSuffixRoute(".scss", "scss.json");
        registerSuffixRoute(".less", "less.json");
        registerSuffixRoute(".cmake", "cmake.json");
        registerSuffixRoute(".dockerfile", "dockerfile.json");
        registerSuffixRoute(".mk", "makefile.json");
        registerSuffixRoute(".properties", "properties.json");
        registerSuffixRoute(".env", "env.json");
        registerSuffixRoute(".proto", "protobuf.json");
        registerSuffixRoute(".graphql", "graphql.json");
        registerSuffixRoute(".gql", "graphql.json");
        registerSuffixRoute(".nginx", "nginx.json");
        registerSuffixRoute(".conf", "nginx.json");
        registerSuffixRoute(".gitignore", "gitignore.json");
        registerSuffixRoute(".diff", "diff.json");
        registerSuffixRoute(".patch", "diff.json");
        registerSuffixRoute(".rb", "ruby.json");
        registerSuffixRoute(".rake", "ruby.json");
        registerSuffixRoute(".gemspec", "ruby.json");
        registerSuffixRoute(".ru", "ruby.json");
        registerSuffixRoute(".hcl", "hcl.json");
        registerSuffixRoute(".tf", "terraform.json");
        registerSuffixRoute(".tfvars", "terraform.json");
        registerSuffixRoute(".tfbackend", "terraform.json");
        registerSuffixRoute(".vue", "vue.json");
        registerSuffixRoute(".svelte", "svelte.json");

        SORTED_SUFFIXES.sort((left, right) -> Integer.compare(right.length(), left.length()));
    }

    private static void registerExactRoute(String fileName, String syntaxFileName, String routedDocumentName) {
        EXACT_SYNTAX_MAP.put(fileName, syntaxFileName);
        if (routedDocumentName != null && !routedDocumentName.isEmpty()) {
            ROUTED_DOCUMENT_NAMES.put(fileName, routedDocumentName);
        }
    }

    private static void registerSuffixRoute(String suffix, String syntaxFileName) {
        SUFFIX_SYNTAX_MAP.put(suffix, syntaxFileName);
        SORTED_SUFFIXES.add(suffix);
    }

    @Nullable
    private static String resolveSyntaxFileName(String fileName) {
        String exact = EXACT_SYNTAX_MAP.get(fileName);
        if (exact != null) {
            return exact;
        }
        for (String suffix : SORTED_SUFFIXES) {
            if (fileName.endsWith(suffix)) {
                return SUFFIX_SYNTAX_MAP.get(suffix);
            }
        }
        return null;
    }

    private static boolean hasSyntaxMapping(String fileName) {
        return resolveSyntaxFileName(fileName) != null;
    }

    @NonNull
    private static String resolveDocumentFileName(String fileName) {
        String routed = ROUTED_DOCUMENT_NAMES.get(fileName);
        return routed != null ? routed : fileName;
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
                .setTitle("Select Highlight Theme")
                .setSingleChoiceItems(names, currentThemeIndex, (dialog, which) -> {
                    currentThemeIndex = which;
                    currentTheme = themes.get(which);
                    applyThemeColors();
                    reHighlightCurrentFile();
                    dialog.dismiss();
                })
                .setNegativeButton("Cancel", null)
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
                    if (!name.startsWith("example")
                            && !EXACT_SAMPLE_FILES.contains(name)
                            && !name.equals("json-sweetline.json")) {
                        continue;
                    }
                    if (hasSyntaxMapping(name)) {
                        files.add(name);
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to list assets files", e);
        }
        return files;
    }

    private void highlightFile(String testFileName) {
        String syntaxFileName = resolveSyntaxFileName(testFileName);
        if (syntaxFileName == null || syntaxFileName.isEmpty()) {
            Log.w(TAG, "Syntax rule file not found for the given test file");
            return;
        }
        boolean inlineStyle = INLINE_STYLE_FILES.contains(syntaxFileName);
        highlight(testFileName, syntaxFileName, resolveDocumentFileName(testFileName), inlineStyle);
    }

    private void highlight(String testFileName, String syntaxFileName, String documentFileName, boolean inlineStyle) {
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
            Document document = new Document(documentFileName, testCode);

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
