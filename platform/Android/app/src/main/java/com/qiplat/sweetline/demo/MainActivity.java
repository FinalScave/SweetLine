package com.qiplat.sweetline.demo;

import android.content.res.ColorStateList;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.text.Editable;
import android.text.Spannable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.ColorUtils;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.HighlightConfig;
import com.qiplat.sweetline.HighlightEngine;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.SyntaxCompileError;
import com.qiplat.sweetline.util.AssetUtils;
import com.google.android.material.card.MaterialCardView;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class MainActivity extends AppCompatActivity implements SpannableStyleFactory {
    private static final String TAG = "SampleHighlight";
    private static final String SYNTAX_ASSET_DIR = "syntaxes";
    private static final String EXAMPLE_ASSET_DIR = "examples";
    private static final String SYNTAX_SAMPLE_NAME = "json-sweetline.json";
    private static final HighlightEngine inlineStyleEngine = new HighlightEngine(new HighlightConfig(true, true));
    private static final HighlightEngine commonEngine = new HighlightEngine(new HighlightConfig(true, false));
    private static final List<String> INLINE_STYLE_SAMPLE_FILES = Arrays.asList(
            "example.java",
            "example.t"
    );
    private static final List<String> INLINE_STYLE_SYNTAX_FILES = Arrays.asList(
            "java-inlineStyle.json",
            "tiecode-inlineStyle.json"
    );

    private static boolean commonEngineConfigured = false;
    private static boolean inlineStyleEngineConfigured = false;
    private static boolean commonSyntaxesCompiled = false;
    private static boolean inlineSyntaxesCompiled = false;

    private View rootView;
    private MaterialCardView topBarCard;
    private MaterialCardView editorCard;
    private View compileStatusContainer;
    private View fileSelectorContainer;
    private Spinner fileSpinner;
    private MyEditText spanText;
    private ProgressBar compileProgress;
    private TextView compileStatus;
    private TextView themeLabel;
    private CharSequence previousText;
    private DocumentAnalyzer analyzer;
    private boolean shouldAnalyzeChange = false;
    private List<DemoSample> demoSamples = new ArrayList<>();

    private List<HighlightTheme> themes;
    private int currentThemeIndex = 0;
    private HighlightTheme currentTheme;

    private static final class DemoSample {
        final String displayName;
        final String assetFileName;
        final boolean inlineStyle;

        DemoSample(String displayName, String assetFileName, boolean inlineStyle) {
            this.displayName = displayName;
            this.assetFileName = assetFileName;
            this.inlineStyle = inlineStyle;
        }

        @NonNull
        @Override
        public String toString() {
            return displayName;
        }
    }

    @FunctionalInterface
    private interface CompileProgressListener {
        void onCompiling(String syntaxFileName, int compiledCount, int totalCount);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        rootView = findViewById(R.id.main);
        topBarCard = findViewById(R.id.top_bar_card);
        editorCard = findViewById(R.id.editor_card);
        compileStatusContainer = findViewById(R.id.compile_status_container);
        fileSelectorContainer = findViewById(R.id.file_selector_container);
        fileSpinner = findViewById(R.id.file_spinner);
        spanText = findViewById(R.id.span_text);
        compileProgress = findViewById(R.id.compile_progress);
        compileStatus = findViewById(R.id.compile_status);
        themeLabel = findViewById(R.id.theme_label);

        themes = HighlightTheme.builtinThemes();
        currentTheme = themes.get(currentThemeIndex);
        applyThemeColors();
        setCompileUiState(true, "Preparing syntax rules...");
        fileSpinner.setEnabled(false);
        spanText.setEnabled(false);

        fileSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                DemoSample sample = demoSamples.get(position);
                shouldAnalyzeChange = false;
                highlightSample(sample);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        spanText.setOnSelectionChangeListener((startIndex, endIndex) ->
                Log.i(TAG, String.format("start: %d, end: %d", startIndex, endIndex)));
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

        startPreCompileDemoSyntaxes();
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
        int backgroundColor = currentTheme.backgroundColor;
        int textColor = currentTheme.textColor;
        boolean darkBackground = ColorUtils.calculateLuminance(backgroundColor) < 0.3;
        int toolbarSurfaceColor = blendColor(backgroundColor, darkBackground ? Color.WHITE : Color.BLACK,
                darkBackground ? 0.08f : 0.04f);
        int editorSurfaceColor = blendColor(backgroundColor, darkBackground ? Color.BLACK : Color.WHITE,
                darkBackground ? 0.08f : 0.02f);
        int strokeColor = ColorUtils.setAlphaComponent(
                blendColor(textColor, backgroundColor, 0.45f), darkBackground ? 110 : 60);
        int mutedTextColor = blendColor(textColor, backgroundColor, 0.35f);
        int keywordColor = currentTheme.colorMap.get(HighlightTheme.STYLE_KEYWORD, textColor);
        int punctuationColor = currentTheme.colorMap.get(HighlightTheme.STYLE_PUNCTUATION, keywordColor);
        int chipColor = blendColor(backgroundColor, keywordColor, darkBackground ? 0.28f : 0.18f);
        int statusColor = blendColor(toolbarSurfaceColor, punctuationColor, darkBackground ? 0.22f : 0.12f);
        int selectorColor = blendColor(toolbarSurfaceColor, darkBackground ? Color.WHITE : Color.BLACK,
                darkBackground ? 0.08f : 0.03f);

        rootView.setBackgroundColor(blendColor(backgroundColor,
                darkBackground ? Color.BLACK : Color.WHITE, darkBackground ? 0.08f : 0.02f));
        spanText.setTextColor(currentTheme.textColor);
        spanText.setHintTextColor(mutedTextColor);

        applyCardStyle(topBarCard, toolbarSurfaceColor, strokeColor);
        applyCardStyle(editorCard, editorSurfaceColor, strokeColor);
        compileStatusContainer.setBackground(createRoundedBackground(statusColor, strokeColor, 12));
        fileSelectorContainer.setBackground(createRoundedBackground(selectorColor, strokeColor, 10));
        themeLabel.setBackground(createRoundedBackground(chipColor, strokeColor, 12));
        compileProgress.setIndeterminateTintList(ColorStateList.valueOf(keywordColor));

        if (getSupportActionBar() != null) {
            getSupportActionBar().setSubtitle(currentTheme.name);
        }

        themeLabel.setText(currentTheme.name);
        themeLabel.setTextColor(textColor);
        compileStatus.setTextColor(textColor);

        int statusBarColor = darkenColor(backgroundColor, 0.7f);
        getWindow().setStatusBarColor(statusBarColor);

        if (getSupportActionBar() != null) {
            getSupportActionBar().setBackgroundDrawable(new ColorDrawable(statusBarColor));
        }
    }

    private void applyCardStyle(MaterialCardView cardView, int backgroundColor, int strokeColor) {
        cardView.setCardBackgroundColor(backgroundColor);
        cardView.setStrokeColor(strokeColor);
        cardView.setStrokeWidth(dpToPx(1));
    }

    private GradientDrawable createRoundedBackground(int fillColor, int strokeColor, int radiusDp) {
        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(fillColor);
        drawable.setCornerRadius(dpToPx(radiusDp));
        if (strokeColor != Color.TRANSPARENT) {
            drawable.setStroke(dpToPx(1), strokeColor);
        }
        return drawable;
    }

    private int blendColor(int fromColor, int toColor, float ratio) {
        return ColorUtils.blendARGB(fromColor, toColor, ratio);
    }

    private int dpToPx(int dp) {
        return Math.round(getResources().getDisplayMetrics().density * dp);
    }

    private void setCompileUiState(boolean compiling, CharSequence statusText) {
        compileProgress.setVisibility(compiling ? View.VISIBLE : View.GONE);
        compileStatus.setText(statusText);
        fileSpinner.setEnabled(!compiling);
        spanText.setEnabled(!compiling);
    }

    private void postCompileStatus(boolean compiling, CharSequence statusText) {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) {
                return;
            }
            setCompileUiState(compiling, statusText);
        });
    }

    private void startPreCompileDemoSyntaxes() {
        new Thread(() -> {
            try {
                long startedAt = System.nanoTime();
                List<String> commonSyntaxFiles = listCommonSyntaxFiles();
                int totalSyntaxFileCount = commonSyntaxFiles.size() + INLINE_STYLE_SYNTAX_FILES.size();
                int compiledCount = ensureCommonSyntaxesCompiled(commonSyntaxFiles, 0,
                        totalSyntaxFileCount, this::onCompileProgress);
                compiledCount = ensureInlineSyntaxesCompiled(compiledCount,
                        totalSyntaxFileCount, this::onCompileProgress);
                final int finalCompiledCount = compiledCount;
                final long elapsedMillis = (System.nanoTime() - startedAt) / 1_000_000;
                runOnUiThread(() -> {
                    if (isFinishing() || isDestroyed()) {
                        return;
                    }
                    demoSamples = listDemoSamples();
                    ArrayAdapter<DemoSample> adapter = new ArrayAdapter<>(this,
                            android.R.layout.simple_spinner_item, demoSamples);
                    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                    fileSpinner.setAdapter(adapter);
                    setCompileUiState(false,
                            "Compiled " + finalCompiledCount + " syntax rule files in "
                                    + elapsedMillis + " ms");
                });
            } catch (IOException | SyntaxCompileError e) {
                Log.e(TAG, "Failed to pre-compile demo syntaxes", e);
                postCompileStatus(false, "Failed to compile syntax rules: " + e.getMessage());
            }
        }, "sweetline-demo-compile").start();
    }

    private void onCompileProgress(String syntaxFileName, int compiledCount, int totalCount) {
        postCompileStatus(true,
                "Compiling " + (compiledCount + 1) + "/" + totalCount + ": " + syntaxFileName);
    }

    private void reHighlightCurrentFile() {
        int pos = fileSpinner.getSelectedItemPosition();
        if (pos >= 0 && pos < demoSamples.size()) {
            shouldAnalyzeChange = false;
            highlightSample(demoSamples.get(pos));
        }
    }

    private static int darkenColor(int color, float factor) {
        int a = Color.alpha(color);
        int r = Math.round(Color.red(color) * factor);
        int g = Math.round(Color.green(color) * factor);
        int b = Math.round(Color.blue(color) * factor);
        return Color.argb(a, Math.min(r, 255), Math.min(g, 255), Math.min(b, 255));
    }

    private static void configureCommonEngine() {
        if (commonEngineConfigured) {
            return;
        }
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
        commonEngine.defineMacro("ANDROID");
        commonEngineConfigured = true;
    }

    private static void configureInlineStyleEngine() {
        if (inlineStyleEngineConfigured) {
            return;
        }
        inlineStyleEngine.defineMacro("ANDROID");
        inlineStyleEngineConfigured = true;
    }

    private int ensureCommonSyntaxesCompiled(List<String> syntaxFiles, int compiledCount,
                                             int totalCount, CompileProgressListener progressListener)
            throws IOException, SyntaxCompileError {
        configureCommonEngine();
        if (commonSyntaxesCompiled) {
            return compiledCount + syntaxFiles.size();
        }
        long start = System.nanoTime();
        int finalCompiledCount = compileSyntaxFiles(commonEngine, syntaxFiles, compiledCount,
                totalCount, progressListener);
        long end = System.nanoTime();
        commonSyntaxesCompiled = true;
        Log.i(TAG, String.format("====preCompileCommonSyntaxes: %dus, count=%d",
                (end - start) / 1000, syntaxFiles.size()));
        return finalCompiledCount;
    }

    private int ensureInlineSyntaxesCompiled(int compiledCount, int totalCount,
                                             CompileProgressListener progressListener)
            throws IOException, SyntaxCompileError {
        configureInlineStyleEngine();
        if (inlineSyntaxesCompiled) {
            return compiledCount + INLINE_STYLE_SYNTAX_FILES.size();
        }
        List<String> syntaxFiles = new ArrayList<>(INLINE_STYLE_SYNTAX_FILES);
        long start = System.nanoTime();
        int finalCompiledCount = compileSyntaxFiles(inlineStyleEngine, syntaxFiles, compiledCount,
                totalCount, progressListener);
        long end = System.nanoTime();
        inlineSyntaxesCompiled = true;
        Log.i(TAG, String.format("====preCompileInlineSyntaxes: %dus, count=%d",
                (end - start) / 1000, syntaxFiles.size()));
        return finalCompiledCount;
    }

    private List<String> listCommonSyntaxFiles() throws IOException {
        List<String> files = new ArrayList<>();
        String[] allAssets = getAssets().list(SYNTAX_ASSET_DIR);
        if (allAssets == null) {
            return files;
        }
        for (String name : allAssets) {
            if (!name.endsWith(".json") || name.endsWith("-inlineStyle.json")) {
                continue;
            }
            files.add(name);
        }
        return files;
    }

    private int compileSyntaxFiles(HighlightEngine engine, List<String> syntaxFiles, int compiledCount,
                                   int totalCount, CompileProgressListener progressListener)
            throws IOException, SyntaxCompileError {
        List<String> pending = new ArrayList<>(syntaxFiles);
        Collections.sort(pending);
        while (!pending.isEmpty()) {
            boolean madeProgress = false;
            List<String> nextPending = new ArrayList<>();
            for (String syntaxFile : pending) {
                progressListener.onCompiling(syntaxFile, compiledCount, totalCount);
                String json = AssetUtils.readAsset(this, SYNTAX_ASSET_DIR + "/" + syntaxFile);
                try {
                    engine.compileSyntaxFromJson(json);
                    madeProgress = true;
                    compiledCount++;
                } catch (SyntaxCompileError e) {
                    if (e.getErrorCode() == SyntaxCompileError.ERR_IMPORT_SYNTAX_NOT_FOUND) {
                        nextPending.add(syntaxFile);
                        continue;
                    }
                    Log.e(TAG, "Failed to compile syntax " + syntaxFile
                            + ", errorCode=" + e.getErrorCode()
                            + ", message=" + e.getMessage(), e);
                    throw e;
                }
            }
            if (!madeProgress && !nextPending.isEmpty()) {
                throw new SyntaxCompileError(
                        SyntaxCompileError.ERR_IMPORT_SYNTAX_NOT_FOUND,
                        "Unresolved syntax dependencies: " + TextUtils.join(", ", nextPending)
                );
            }
            pending = nextPending;
        }
        return compiledCount;
    }

    private List<DemoSample> listDemoSamples() {
        List<DemoSample> samples = new ArrayList<>();
        try {
            String[] exampleAssets = getAssets().list(EXAMPLE_ASSET_DIR);
            if (exampleAssets != null) {
                for (String name : exampleAssets) {
                    samples.add(new DemoSample(name, EXAMPLE_ASSET_DIR + "/" + name, false));
                    if (INLINE_STYLE_SAMPLE_FILES.contains(name)) {
                        samples.add(new DemoSample(name + " [inline]", EXAMPLE_ASSET_DIR + "/" + name, true));
                    }
                }
            }

            String[] syntaxAssets = getAssets().list(SYNTAX_ASSET_DIR);
            if (syntaxAssets != null) {
                for (String name : syntaxAssets) {
                    if (!SYNTAX_SAMPLE_NAME.equals(name)) {
                        continue;
                    }
                    samples.add(new DemoSample(name, SYNTAX_ASSET_DIR + "/" + name, false));
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to list demo samples", e);
        }
        return samples;
    }

    private void highlightSample(DemoSample sample) {
        analyzer = null;
        try {
            HighlightEngine engine = sample.inlineStyle ? inlineStyleEngine : commonEngine;

            String testCode = AssetUtils.readAsset(this, sample.assetFileName);
            Document document = new Document(sample.assetFileName, testCode);

            long start = System.nanoTime();
            analyzer = engine.loadDocument(document);
            long end = System.nanoTime();
            Log.i(TAG, String.format("====loadDocument: %dus", (end - start) / 1000));

            start = System.nanoTime();
            Spannable highlightedText = analyzer != null ? analyzer.analyzeAsSpannable(this) : null;
            end = System.nanoTime();
            Log.i(TAG, String.format("====analyze: %dus", (end - start) / 1000));

            if (highlightedText == null) {
                Log.w(TAG, "No syntax rule resolved for sample: " + sample.assetFileName);
                spanText.setText(testCode);
                return;
            }
            spanText.setText(highlightedText);
        } catch (Exception e) {
            Log.e(TAG, "Failed to highlight sample: " + sample.assetFileName, e);
            spanText.setText("");
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
            Log.d(TAG, "delete: index=" + start
                    + ", length=" + before + ", content='" + deleted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start + before,
                    "", this);
        } else if (before == 0 && count > 0) {
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "insert: index=" + start
                    + ", length=" + count + ", content='" + inserted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start,
                    inserted, this);
        } else if (before > 0 && count > 0) {
            String deleted = oldText.subSequence(start, start + before).toString();
            String inserted = newText.subSequence(start, start + count).toString();
            Log.d(TAG, "replace: index=" + start
                    + ", deleted='" + deleted + "', inserted='" + inserted + "'");
            newHighlightedText = analyzer.analyzeIncrementalAsSpannable(start, start + before,
                    inserted, this);
        }
        long endTime = System.nanoTime();
        Log.i(TAG, String.format("====analyzeChanges: %dus", (endTime - startTime) / 1000));
        shouldAnalyzeChange = false;
        if (newHighlightedText == null) {
            return;
        }
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
