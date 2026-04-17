package com.qiplat.sweetline;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

/**
 * SweetLine highlight engine.
 * <p>
 * The engine is responsible for compiling syntax rules, creating analyzers,
 * and managing documents. Implements {@link AutoCloseable} for deterministic
 * native resource release.
 * <p>
 * Usage example:
 * <pre>{@code
 * try (HighlightEngine engine = new HighlightEngine(new HighlightConfig(true, false))) {
 *     engine.compileSyntaxFromFile("/path/to/java.json");
 *     try (TextAnalyzer analyzer = engine.createAnalyzerBySyntaxName("java")) {
 *         DocumentHighlight result = analyzer.analyzeText("public class Foo {}");
 *     }
 * }
 * }</pre>
 */
public class HighlightEngine implements AutoCloseable {

    private final MemorySegment handle;
    private boolean closed = false;

    /**
     * Create a highlight engine with the given configuration.
     *
     * @param config Highlight configuration
     */
    public HighlightEngine(HighlightConfig config) {
        try {
            this.handle = (MemorySegment) SweetLineNative.sl_create_engine
                    .invoke(config.showIndex(), config.inlineStyle());
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create SweetLine engine", e);
        }
    }

    /**
     * Create a highlight engine with default configuration (no index, no inline style).
     */
    public HighlightEngine() {
        this(new HighlightConfig());
    }

    /**
     * Register a highlight style name mapping.
     *
     * @param styleName Style name
     * @param styleId   Style ID
     */
    public void registerStyleName(String styleName, int styleId) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment nameSeg = arena.allocateFrom(styleName);
            SweetLineNative.sl_engine_register_style_name.invoke(handle, nameSeg, styleId);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to register style name", e);
        }
    }

    /**
     * Get the registered style name by style ID.
     *
     * @param styleId Style ID
     * @return Style name, or null if not found
     */
    public String getStyleName(int styleId) {
        ensureOpen();
        try {
            MemorySegment result = (MemorySegment) SweetLineNative.sl_engine_get_style_name.invoke(handle, styleId);
            if (result.equals(MemorySegment.NULL)) {
                return null;
            }
            return result.reinterpret(Long.MAX_VALUE).getString(0);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to get style name", e);
        }
    }

    /**
     * Define a macro for controlling #ifdef conditional compilation in importSyntax.
     *
     * @param macroName Macro name
     */
    public void defineMacro(String macroName) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment nameSeg = arena.allocateFrom(macroName);
            SweetLineNative.sl_engine_define_macro.invoke(handle, nameSeg);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to define macro", e);
        }
    }

    /**
     * Undefine a macro.
     *
     * @param macroName Macro name
     */
    public void undefineMacro(String macroName) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment nameSeg = arena.allocateFrom(macroName);
            SweetLineNative.sl_engine_undefine_macro.invoke(handle, nameSeg);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to undefine macro", e);
        }
    }

    /**
     * Compile syntax rule from JSON content.
     *
     * @param syntaxJson JSON content of the syntax rule
     * @throws SyntaxCompileError if compilation fails
     */
    public void compileSyntaxFromJson(String syntaxJson) throws SyntaxCompileError {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment jsonSeg = arena.allocateFrom(syntaxJson);
            // Struct-returning FFM calls require a SegmentAllocator as the first implicit argument
            MemorySegment errorStruct = (MemorySegment) SweetLineNative.sl_engine_compile_json.invoke(arena, handle, jsonSeg);
            SweetLineNative.checkSyntaxError(errorStruct);
        } catch (SyntaxCompileError e) {
            throw e;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to compile syntax from JSON", e);
        }
    }

    /**
     * Compile syntax rule from a JSON file.
     *
     * @param path Path to the syntax rule JSON file
     * @throws SyntaxCompileError if compilation fails
     */
    public void compileSyntaxFromFile(String path) throws SyntaxCompileError {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment pathSeg = arena.allocateFrom(path);
            // Struct-returning FFM calls require a SegmentAllocator as the first implicit argument
            MemorySegment errorStruct = (MemorySegment) SweetLineNative.sl_engine_compile_file.invoke(arena, handle, pathSeg);
            SweetLineNative.checkSyntaxError(errorStruct);
        } catch (SyntaxCompileError e) {
            throw e;
        } catch (Throwable e) {
            throw new RuntimeException("Failed to compile syntax from file", e);
        }
    }

    /**
     * Create a text highlight analyzer by syntax rule name.
     * The returned analyzer does not support incremental analysis.
     *
     * @param syntaxName Syntax rule name (e.g. "java")
     * @return Text analyzer, or null if syntax not found
     */
    public TextAnalyzer createAnalyzerBySyntaxName(String syntaxName) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment nameSeg = arena.allocateFrom(syntaxName);
            MemorySegment analyzerHandle = (MemorySegment) SweetLineNative.sl_engine_create_text_analyzer
                    .invoke(handle, nameSeg);
            if (analyzerHandle.equals(MemorySegment.NULL)) {
                return null;
            }
            return new TextAnalyzer(analyzerHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create text analyzer", e);
        }
    }

    /**
     * Create a text highlight analyzer by file name.
     * The returned analyzer does not support incremental analysis.
     *
     * @param fileName File name or basename used for syntax routing
     * @return Text analyzer, or null if syntax not found
     */
    public TextAnalyzer createAnalyzerByFileName(String fileName) {
        ensureOpen();
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment fileNameSeg = arena.allocateFrom(fileName);
            MemorySegment analyzerHandle = (MemorySegment) SweetLineNative.sl_engine_create_text_analyzer_by_file_name
                    .invoke(handle, fileNameSeg);
            if (analyzerHandle.equals(MemorySegment.NULL)) {
                return null;
            }
            return new TextAnalyzer(analyzerHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to create text analyzer by file name", e);
        }
    }

    /**
     * Load a managed document and get a document highlight analyzer (supports incremental analysis).
     *
     * @param document Managed document
     * @return Document analyzer
     */
    public DocumentAnalyzer loadDocument(Document document) {
        ensureOpen();
        try {
            MemorySegment analyzerHandle = (MemorySegment) SweetLineNative.sl_engine_load_document
                    .invoke(handle, document.handle());
            if (analyzerHandle.equals(MemorySegment.NULL)) {
                return null;
            }
            return new DocumentAnalyzer(analyzerHandle);
        } catch (Throwable e) {
            throw new RuntimeException("Failed to load document", e);
        }
    }

    @Override
    public void close() {
        if (!closed) {
            closed = true;
            try {
                SweetLineNative.sl_free_engine.invoke(handle);
            } catch (Throwable e) {
                throw new RuntimeException("Failed to free SweetLine engine", e);
            }
        }
    }

    private void ensureOpen() {
        if (closed) {
            throw new IllegalStateException("HighlightEngine is already closed");
        }
    }
}
