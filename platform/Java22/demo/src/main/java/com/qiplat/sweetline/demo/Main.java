package com.qiplat.sweetline.demo;

import com.qiplat.sweetline.*;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * SweetLine Java 22 FFM Swing demo application.
 * Supports selecting built-in syntax / example files and switching highlight themes
 */
public class Main extends JFrame {

    private static final Map<String, String> EXT_SYNTAX_MAP = new LinkedHashMap<>();

    static {
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
        EXT_SYNTAX_MAP.put(".m", "objc.json");
        EXT_SYNTAX_MAP.put(".php", "php.json");
        EXT_SYNTAX_MAP.put(".ps1", "powershell.json");
        EXT_SYNTAX_MAP.put(".py", "python.json");
        EXT_SYNTAX_MAP.put(".rs", "rust.json");
        EXT_SYNTAX_MAP.put(".scala", "scala.json");
        EXT_SYNTAX_MAP.put(".sh", "shell.json");
        EXT_SYNTAX_MAP.put(".sql", "sql.json");
        EXT_SYNTAX_MAP.put(".swift", "swift.json");
        EXT_SYNTAX_MAP.put(".toml", "toml.json");
        EXT_SYNTAX_MAP.put(".ts", "typescript.json");
        EXT_SYNTAX_MAP.put(".vb", "vb.json");
        EXT_SYNTAX_MAP.put(".xml", "xml.json");
        EXT_SYNTAX_MAP.put(".yaml", "yaml.json");
        EXT_SYNTAX_MAP.put(".md", "markdown.json");
        EXT_SYNTAX_MAP.put(".wenyan", "wenyan.json");
        EXT_SYNTAX_MAP.put(".myu", "iapp.json");
    }

    private Path syntaxesDir;
    private Path examplesDir;
    private List<String> exampleFiles;
    private List<HighlightTheme> themes;

    private HighlightEngine engine;
    private JComboBox<String> fileCombo;
    private JComboBox<String> themeCombo;
    private JLabel statusLabel;
    private CodePanel codePanel;

    private HighlightTheme currentTheme;

    private boolean suppressComboEvents = false;

    public Main(Path syntaxesDir, Path examplesDir) {
        super("SweetLine Demo(Swing)");
        this.syntaxesDir = syntaxesDir;
        this.examplesDir = examplesDir;
        this.themes = HighlightTheme.builtinThemes();
        this.currentTheme = themes.getFirst();
        this.exampleFiles = listExampleFiles(examplesDir);

        this.engine = new HighlightEngine(new HighlightConfig(true, false));
        registerStyleNames(engine);

        initUI();
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setSize(640, 480);
        setLocationRelativeTo(null);

        if (!exampleFiles.isEmpty()) {
            fileCombo.setSelectedIndex(0);
        }
    }

    private void initUI() {
        JPanel toolbar = new JPanel(new FlowLayout(FlowLayout.LEFT, 8, 4));

        toolbar.add(new JLabel("File:"));
        fileCombo = new JComboBox<>(exampleFiles.toArray(new String[0]));
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

        JButton openBtn = new JButton("Open...");
        openBtn.addActionListener(e -> openExternalFile());
        toolbar.add(openBtn);

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

    private void highlightSelectedFile() {
        int idx = fileCombo.getSelectedIndex();
        if (idx < 0 || idx >= exampleFiles.size()) {
            return;
        }
        String fileName = exampleFiles.get(idx);
        String ext = getFileExtension(fileName);
        String syntaxFileName = EXT_SYNTAX_MAP.get(ext);
        if (syntaxFileName == null) {
            statusLabel.setText("No syntax mapping for extension: " + ext);
            return;
        }

        Path syntaxPath = syntaxesDir.resolve(syntaxFileName);
        Path examplePath = examplesDir.resolve(fileName);

        if (!Files.exists(syntaxPath)) {
            statusLabel.setText("Syntax file not found: " + syntaxPath);
            return;
        }

        try {
            highlightFile(examplePath, syntaxPath);
        } catch (Exception ex) {
            statusLabel.setText("Error: " + ex.getMessage());
        }
    }

    private void highlightFile(Path filePath, Path syntaxPath) throws IOException, SyntaxCompileError {
        String sourceCode = Files.readString(filePath);
        String syntaxJson = Files.readString(syntaxPath);
        String fileName = filePath.getFileName().toString();

        long compileStart = System.nanoTime();
        engine.compileSyntaxFromJson(syntaxJson);
        long compileUs = (System.nanoTime() - compileStart) / 1000;

        try (Document doc = new Document(fileName, sourceCode)) {
            DocumentAnalyzer analyzer = engine.loadDocument(doc);
            if (analyzer == null) {
                statusLabel.setText("Failed to load document.");
                return;
            }

            long analyzeStart = System.nanoTime();
            DocumentHighlight highlight = analyzer.analyze();
            long analyzeUs = (System.nanoTime() - analyzeStart) / 1000;

            IndentGuideResult indentGuides = analyzer.analyzeIndentGuides();

            int lineCount = sourceCode.split("\n", -1).length;
            statusLabel.setText(String.format("Compile: %d\u00b5s | Analyze: %d\u00b5s | Lines: %d | File: %s",
                    compileUs, analyzeUs, lineCount, fileName));

            codePanel.setHighlightData(sourceCode, highlight, indentGuides);
            codePanel.revalidate();
            codePanel.repaint();

            analyzer.close();
        }
    }

    private void openExternalFile() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new FileNameExtensionFilter("Source files", EXT_SYNTAX_MAP.keySet().stream()
                .map(ext -> ext.substring(1)).toArray(String[]::new)));
        if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) {
            return;
        }

        Path filePath = chooser.getSelectedFile().toPath();
        String ext = getFileExtension(filePath.getFileName().toString());
        String syntaxFileName = EXT_SYNTAX_MAP.get(ext);
        if (syntaxFileName == null) {
            statusLabel.setText("Unsupported file extension: " + ext);
            return;
        }

        Path syntaxPath = syntaxesDir.resolve(syntaxFileName);
        if (!Files.exists(syntaxPath)) {
            statusLabel.setText("Syntax file not found: " + syntaxPath);
            return;
        }

        try {
            suppressComboEvents = true;
            fileCombo.setSelectedIndex(-1);
            suppressComboEvents = false;

            highlightFile(filePath, syntaxPath);
        } catch (Exception ex) {
            statusLabel.setText("Error: " + ex.getMessage());
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
    }

    private static List<String> listExampleFiles(Path dir) {
        List<String> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir)) {
            for (Path entry : stream) {
                String name = entry.getFileName().toString();
                if (!name.startsWith("example") && !name.equals("json-sweetline.json")) {
                    continue;
                }
                String ext = getFileExtension(name);
                if (EXT_SYNTAX_MAP.containsKey(ext)) {
                    files.add(name);
                }
            }
        } catch (IOException e) {
            System.err.println("Failed to list example files: " + e.getMessage());
        }
        files.sort(Comparator.naturalOrder());
        return files;
    }

    private static String getFileExtension(String fileName) {
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex >= 0) {
            return fileName.substring(dotIndex);
        }
        return "";
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