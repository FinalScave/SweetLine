package com.qiplat.sweetline.demo;

import com.qiplat.sweetline.Document;
import com.qiplat.sweetline.DocumentAnalyzer;
import com.qiplat.sweetline.DocumentHighlight;
import com.qiplat.sweetline.HighlightConfig;
import com.qiplat.sweetline.HighlightEngine;
import com.qiplat.sweetline.IndentGuideResult;
import com.qiplat.sweetline.SyntaxCompileError;
import com.qiplat.sweetline.TextAnalyzer;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.SwingWorker;
import javax.swing.UIManager;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * SweetLine Java 22 FFM Swing demo application.
 * Supports selecting built-in syntax / example files and switching highlight themes.
 */
public class Main extends JFrame {

    private static final String WINDOWS_MACRO = "WINDOWS";
    private static final String YAML_NON_ZERO_WIDTH_FILE = "yaml(non zero width).json";
    private static final String SYNTAX_SAMPLE_FILE = "json-sweetline.json";

    private record SyntaxSource(String fileName, String json) {
    }

    private record WarmupProgress(int compiledCount, int totalCount, String fileName) {
    }

    private record WarmupResult(List<String> exampleFiles, int compiledCount, long elapsedMillis) {
    }

    private final Path syntaxesDir;
    private final Path examplesDir;
    private List<String> exampleFiles;
    private final List<HighlightTheme> themes;

    private final HighlightEngine engine;
    private JComboBox<String> fileCombo;
    private JComboBox<String> themeCombo;
    private JButton openButton;
    private JLabel statusLabel;
    private CodePanel codePanel;

    private HighlightTheme currentTheme;
    private int compiledSyntaxCount = 0;
    private long warmupElapsedMillis = 0;

    private boolean suppressComboEvents = false;

    public Main(Path syntaxesDir, Path examplesDir) {
        super("SweetLine Demo(Swing)");
        this.syntaxesDir = syntaxesDir;
        this.examplesDir = examplesDir;
        this.exampleFiles = new ArrayList<>();
        this.themes = HighlightTheme.builtinThemes();
        this.currentTheme = themes.getFirst();

        this.engine = new HighlightEngine(new HighlightConfig(true, false));
        registerStyleNames(engine);
        engine.defineMacro(WINDOWS_MACRO);

        initUI();
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setSize(532, 856);
        setLocationRelativeTo(null);

        setWarmupUiState(true);
        statusLabel.setText("Preparing syntax rules...");
        startWarmup();
    }

    private void initUI() {
        JPanel toolbar = new JPanel(new FlowLayout(FlowLayout.LEFT, 8, 4));

        toolbar.add(new JLabel("File:"));
        fileCombo = new JComboBox<>();
        fileCombo.setMaximumRowCount(20);
        fileCombo.addActionListener(e -> {
            if (!suppressComboEvents) {
                highlightSelectedFile();
            }
        });
        toolbar.add(fileCombo);

        toolbar.add(Box.createHorizontalStrut(16));

        toolbar.add(new JLabel("Theme:"));
        themeCombo = new JComboBox<>(HighlightTheme.themeNames());
        themeCombo.addActionListener(e -> {
            if (!suppressComboEvents) {
                int idx = themeCombo.getSelectedIndex();
                if (idx >= 0 && idx < themes.size()) {
                    currentTheme = themes.get(idx);
                    codePanel.setTheme(currentTheme);
                    codePanel.repaint();
                }
            }
        });
        toolbar.add(themeCombo);

        toolbar.add(Box.createHorizontalStrut(16));

        openButton = new JButton("Open...");
        openButton.addActionListener(e -> openExternalFile());
        toolbar.add(openButton);

        codePanel = new CodePanel();
        codePanel.setTheme(currentTheme);
        JScrollPane scrollPane = new JScrollPane(codePanel);
        scrollPane.getVerticalScrollBar().setUnitIncrement(16);
        scrollPane.getHorizontalScrollBar().setUnitIncrement(16);

        statusLabel = new JLabel(" ");
        statusLabel.setBorder(BorderFactory.createEmptyBorder(4, 8, 4, 8));

        getContentPane().setLayout(new BorderLayout());
        getContentPane().add(toolbar, BorderLayout.NORTH);
        getContentPane().add(scrollPane, BorderLayout.CENTER);
        getContentPane().add(statusLabel, BorderLayout.SOUTH);
    }

    private void setWarmupUiState(boolean warmingUp) {
        fileCombo.setEnabled(!warmingUp && !exampleFiles.isEmpty());
        openButton.setEnabled(!warmingUp);
    }

    private void startWarmup() {
        SwingWorker<WarmupResult, WarmupProgress> worker = new SwingWorker<>() {
            @Override
            protected WarmupResult doInBackground() throws Exception {
                long startedAt = System.nanoTime();
                List<SyntaxSource> syntaxSources = listCommonSyntaxSources(syntaxesDir);
                publish(new WarmupProgress(0, syntaxSources.size(), null));
                int compiledCount = compileSyntaxSources(syntaxSources, progress -> publish(progress));
                List<String> demoFiles = listExampleFiles(examplesDir, syntaxesDir);
                long elapsedMillis = (System.nanoTime() - startedAt) / 1_000_000;
                return new WarmupResult(demoFiles, compiledCount, elapsedMillis);
            }

            @Override
            protected void process(List<WarmupProgress> chunks) {
                WarmupProgress progress = chunks.get(chunks.size() - 1);
                if (progress.fileName() == null) {
                    statusLabel.setText(String.format("Compiling 0/%d syntax rule files...", progress.totalCount()));
                    return;
                }
                statusLabel.setText(String.format(
                        "Compiling %d/%d: %s",
                        progress.compiledCount(),
                        progress.totalCount(),
                        progress.fileName()));
            }

            @Override
            protected void done() {
                try {
                    applyWarmupResult(get());
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    handleWarmupFailure(e);
                } catch (ExecutionException e) {
                    handleWarmupFailure(e.getCause() != null ? e.getCause() : e);
                }
            }
        };
        worker.execute();
    }

    private void applyWarmupResult(WarmupResult result) {
        exampleFiles = new ArrayList<>(result.exampleFiles());
        compiledSyntaxCount = result.compiledCount();
        warmupElapsedMillis = result.elapsedMillis();

        suppressComboEvents = true;
        fileCombo.setModel(new DefaultComboBoxModel<>(exampleFiles.toArray(new String[0])));
        fileCombo.setSelectedIndex(-1);
        suppressComboEvents = false;

        setWarmupUiState(false);
        if (exampleFiles.isEmpty()) {
            statusLabel.setText(String.format(
                    "Compiled %d syntax rule files in %d ms | No demo files available",
                    compiledSyntaxCount,
                    warmupElapsedMillis));
            return;
        }

        fileCombo.setSelectedIndex(0);
    }

    private void handleWarmupFailure(Throwable error) {
        setWarmupUiState(true);
        statusLabel.setText("Warmup failed: " + error.getMessage());
        error.printStackTrace();
    }

    private void highlightSelectedFile() {
        int idx = fileCombo.getSelectedIndex();
        if (idx < 0 || idx >= exampleFiles.size()) {
            return;
        }

        String fileName = exampleFiles.get(idx);
        Path examplePath = resolveDemoSamplePath(fileName);
        if (!Files.exists(examplePath)) {
            statusLabel.setText("Example file not found: " + examplePath);
            return;
        }

        try {
            highlightFile(examplePath, fileName);
        } catch (Exception ex) {
            statusLabel.setText("Error: " + ex.getMessage());
        }
    }

    private void highlightFile(Path filePath, String documentFileName) throws IOException {
        String sourceCode = Files.readString(filePath);
        String fileName = filePath.getFileName().toString();

        try (Document doc = new Document(documentFileName, sourceCode);
             DocumentAnalyzer analyzer = engine.loadDocument(doc)) {
            if (analyzer == null) {
                statusLabel.setText("No matching syntax for file: " + documentFileName);
                return;
            }

            long analyzeStart = System.nanoTime();
            DocumentHighlight highlight = analyzer.analyze();
            long analyzeUs = (System.nanoTime() - analyzeStart) / 1000;

            IndentGuideResult indentGuides = analyzer.analyzeIndentGuides();
            int lineCount = sourceCode.split("\n", -1).length;
            statusLabel.setText(String.format(
                    "Warmup: %d files in %d ms | Analyze: %d\u00b5s | Lines: %d | File: %s",
                    compiledSyntaxCount,
                    warmupElapsedMillis,
                    analyzeUs,
                    lineCount,
                    fileName));

            codePanel.setHighlightData(sourceCode, highlight, indentGuides);
            codePanel.revalidate();
            codePanel.repaint();
        }
    }

    private void openExternalFile() {
        JFileChooser chooser = new JFileChooser();
        if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) {
            return;
        }

        Path filePath = chooser.getSelectedFile().toPath();
        String fileName = filePath.getFileName().toString();

        try {
            suppressComboEvents = true;
            fileCombo.setSelectedIndex(-1);
            suppressComboEvents = false;

            highlightFile(filePath, fileName);
        } catch (Exception ex) {
            statusLabel.setText("Error: " + ex.getMessage());
        }
    }

    private List<SyntaxSource> listCommonSyntaxSources(Path dir) throws IOException {
        List<Path> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir, "*.json")) {
            for (Path entry : stream) {
                if (!Files.isRegularFile(entry)) {
                    continue;
                }
                String name = entry.getFileName().toString();
                if (!shouldPrecompileSyntaxFile(name)) {
                    continue;
                }
                files.add(entry);
            }
        }

        files.sort(Comparator.comparing(path -> path.getFileName().toString()));

        List<SyntaxSource> sources = new ArrayList<>(files.size());
        for (Path file : files) {
            sources.add(new SyntaxSource(file.getFileName().toString(), Files.readString(file)));
        }
        return sources;
    }

    private int compileSyntaxSources(List<SyntaxSource> syntaxSources, Consumer<WarmupProgress> onProgress)
            throws SyntaxCompileError {
        List<SyntaxSource> pending = new ArrayList<>(syntaxSources);
        int compiledCount = 0;
        while (!pending.isEmpty()) {
            boolean progressed = false;
            List<SyntaxSource> nextPending = new ArrayList<>();
            for (SyntaxSource source : pending) {
                try {
                    engine.compileSyntaxFromJson(source.json());
                    compiledCount += 1;
                    progressed = true;
                    onProgress.accept(new WarmupProgress(compiledCount, syntaxSources.size(), source.fileName()));
                } catch (SyntaxCompileError e) {
                    if (e.getErrorCode() == SyntaxCompileError.ERR_IMPORT_SYNTAX_NOT_FOUND) {
                        nextPending.add(source);
                        continue;
                    }
                    throw new SyntaxCompileError(
                            e.getErrorCode(),
                            "Failed to compile " + source.fileName() + ": " + e.getMessage());
                }
            }

            if (!progressed) {
                String unresolved = nextPending.stream()
                        .map(SyntaxSource::fileName)
                        .collect(Collectors.joining(", "));
                throw new IllegalStateException("Unresolved importSyntax dependencies: " + unresolved);
            }
            pending = nextPending;
        }
        return compiledCount;
    }

    private boolean shouldPrecompileSyntaxFile(String fileName) {
        return !fileName.endsWith("-inlineStyle.json") && !fileName.equals(YAML_NON_ZERO_WIDTH_FILE);
    }

    private List<String> listExampleFiles(Path examplesDir, Path syntaxesDir) throws IOException {
        List<String> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(examplesDir)) {
            for (Path entry : stream) {
                if (!Files.isRegularFile(entry)) {
                    continue;
                }
                String name = entry.getFileName().toString();
                if (!supportsFileNameRouting(name)) {
                    continue;
                }
                files.add(name);
            }
        }
        Path syntaxSamplePath = syntaxesDir.resolve(SYNTAX_SAMPLE_FILE);
        if (Files.isRegularFile(syntaxSamplePath) && supportsFileNameRouting(SYNTAX_SAMPLE_FILE)) {
            files.add(SYNTAX_SAMPLE_FILE);
        }
        files.sort(Comparator.naturalOrder());
        return files;
    }

    private Path resolveDemoSamplePath(String fileName) {
        Path examplePath = examplesDir.resolve(fileName);
        if (Files.isRegularFile(examplePath)) {
            return examplePath;
        }
        return syntaxesDir.resolve(fileName);
    }

    private boolean supportsFileNameRouting(String fileName) {
        try (TextAnalyzer analyzer = engine.createAnalyzerByFileName(fileName)) {
            return analyzer != null;
        }
    }

    private static void registerStyleNames(HighlightEngine engine) {
        engine.registerStyleName("keyword", HighlightTheme.STYLE_KEYWORD);
        engine.registerStyleName("string", HighlightTheme.STYLE_STRING);
        engine.registerStyleName("number", HighlightTheme.STYLE_NUMBER);
        engine.registerStyleName("comment", HighlightTheme.STYLE_COMMENT);
        engine.registerStyleName("class", HighlightTheme.STYLE_CLASS);
        engine.registerStyleName("method", HighlightTheme.STYLE_METHOD);
        engine.registerStyleName("variable", HighlightTheme.STYLE_VARIABLE);
        engine.registerStyleName("punctuation", HighlightTheme.STYLE_PUNCTUATION);
        engine.registerStyleName("annotation", HighlightTheme.STYLE_ANNOTATION);
        engine.registerStyleName("preprocessor", HighlightTheme.STYLE_PREPROCESSOR);
        engine.registerStyleName("macro", HighlightTheme.STYLE_MACRO);
        engine.registerStyleName("lifetime", HighlightTheme.STYLE_LIFETIME);
        engine.registerStyleName("selector", HighlightTheme.STYLE_SELECTOR);
        engine.registerStyleName("builtin", HighlightTheme.STYLE_BUILTIN);
        engine.registerStyleName("url", HighlightTheme.STYLE_URL);
        engine.registerStyleName("property", HighlightTheme.STYLE_PROPERTY);
    }

    private static Path resolveDir(String[] args, int index, String defaultRelative) {
        if (args.length > index) {
            return Path.of(args[index]).toAbsolutePath().normalize();
        }
        Path candidate = Path.of("../../" + defaultRelative).toAbsolutePath().normalize();
        if (Files.isDirectory(candidate)) {
            return candidate;
        }
        return Path.of(defaultRelative).toAbsolutePath().normalize();
    }

    public static void main(String[] args) {
        Path syntaxesDir = resolveDir(args, 0, "syntaxes");
        Path examplesDir = resolveDir(args, 1, "tests/files");

        if (!Files.isDirectory(syntaxesDir)) {
            System.err.println("Syntaxes directory not found: " + syntaxesDir);
            System.err.println("Usage: Main [syntaxes-dir] [examples-dir]");
            return;
        }
        if (!Files.isDirectory(examplesDir)) {
            System.err.println("Examples directory not found: " + examplesDir);
            System.err.println("Usage: Main [syntaxes-dir] [examples-dir]");
            return;
        }

        SwingUtilities.invokeLater(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            } catch (Exception ignored) {
            }
            new Main(syntaxesDir, examplesDir).setVisible(true);
        });
    }
}
