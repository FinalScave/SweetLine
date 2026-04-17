package com.qiplat.sweetline.demo;

import com.qiplat.sweetline.*;

import javax.swing.*;
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

    private static final Map<String, String> EXACT_SYNTAX_MAP = new LinkedHashMap<>();
    private static final Map<String, String> SUFFIX_SYNTAX_MAP = new LinkedHashMap<>();
    private static final Map<String, String> ROUTED_DOCUMENT_NAMES = new LinkedHashMap<>();
    private static final List<String> SORTED_SUFFIXES = new ArrayList<>();

    static {
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

    private static String resolveDocumentFileName(String fileName) {
        return ROUTED_DOCUMENT_NAMES.getOrDefault(fileName, fileName);
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
        setSize(532, 856);
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
        String syntaxFileName = resolveSyntaxFileName(fileName);
        if (syntaxFileName == null) {
            statusLabel.setText("No syntax mapping for file: " + fileName);
            return;
        }

        Path syntaxPath = syntaxesDir.resolve(syntaxFileName);
        Path examplePath = examplesDir.resolve(fileName);

        if (!Files.exists(syntaxPath)) {
            statusLabel.setText("Syntax file not found: " + syntaxPath);
            return;
        }

        try {
            highlightFile(examplePath, syntaxPath, resolveDocumentFileName(fileName));
        } catch (Exception ex) {
            statusLabel.setText("Error: " + ex.getMessage());
        }
    }

    private void highlightFile(Path filePath, Path syntaxPath, String documentFileName) throws IOException, SyntaxCompileError {
        String sourceCode = Files.readString(filePath);
        String syntaxJson = Files.readString(syntaxPath);
        String fileName = filePath.getFileName().toString();

        long compileStart = System.nanoTime();
        engine.compileSyntaxFromJson(syntaxJson);
        long compileUs = (System.nanoTime() - compileStart) / 1000;

        try (Document doc = new Document(documentFileName, sourceCode)) {
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
        if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) {
            return;
        }

        Path filePath = chooser.getSelectedFile().toPath();
        String fileName = filePath.getFileName().toString();
        String syntaxFileName = resolveSyntaxFileName(fileName);
        if (syntaxFileName == null) {
            statusLabel.setText("Unsupported file name: " + fileName);
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

            highlightFile(filePath, syntaxPath, resolveDocumentFileName(fileName));
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
        engine.registerStyleName("url", HighlightTheme.STYLE_URL);
        engine.registerStyleName("property", HighlightTheme.STYLE_PROPERTY);
    }

    private static List<String> listExampleFiles(Path dir) {
        List<String> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir)) {
            for (Path entry : stream) {
                String name = entry.getFileName().toString();
                if (!name.equals("json-sweetline.json") && resolveSyntaxFileName(name) == null) {
                    continue;
                }
                if (resolveSyntaxFileName(name) != null) {
                    files.add(name);
                }
            }
        } catch (IOException e) {
            System.err.println("Failed to list example files: " + e.getMessage());
        }
        files.sort(Comparator.naturalOrder());
        return files;
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
