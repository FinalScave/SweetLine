package com.qiplat.sweetline;

import java.lang.foreign.*;
import java.lang.invoke.MethodHandle;
import java.nio.file.Files;
import java.nio.file.Path;

import static java.lang.foreign.ValueLayout.*;

/**
 * FFM (Foreign Function & Memory) binding layer for SweetLine native library.
 * Loads libsweetline and declares MethodHandles for all C API functions.
 */
final class SweetLineNative {

    private SweetLineNative() {
    }

    private static final String LIB_PATH_KEY = "sweetline.lib.path";
    private static final String LOAD_LIBRARY_ERROR =
            "Cannot load native library 'sweetline'. " +
                    "Set -Dsweetline.lib.path=<dir> or add the library to java.library.path. ";

    static final Linker LINKER = Linker.nativeLinker();
    static final SymbolLookup LOOKUP;

    static {
        LOOKUP = loadLibraryLookup();
    }

    private static SymbolLookup loadLibraryLookup() {
        String libName = System.mapLibraryName("sweetline");

        // -Dsweetline.lib.path explicitly specified (including the scenario auto-set by NativeLibraryExtractor.extract)
        SymbolLookup lookup = tryExplicitLibrary(libName);
        if (lookup != null) {
            return lookup;
        }

        // Try auto-extracting from JAR resources to default directory and load
        lookup = tryLoadFromJarResources();
        if (lookup != null) {
            return lookup;
        }

        // Fallback to system path (java.library.path)
        return loadLibraryFromSystem();
    }

    /**
     * Try loading from explicitly specified path via -Dsweetline.lib.path.
     */
    private static SymbolLookup tryExplicitLibrary(String libName) {
        String libPath = System.getProperty(LIB_PATH_KEY);
        if (libPath == null || libPath.isBlank()) {
            return null;
        }
        return lookupLibrary(Path.of(libPath, libName));
    }

    /**
     * Try auto-extracting the native library from JAR resources to the default directory (~/.sweetline/native/),
     * automatically set sweetline.lib.path and load after successful extraction.
     * This is the automatic fallback loading method for Maven release scenarios.
     */
    private static SymbolLookup tryLoadFromJarResources() {
        try {
            Path libPath = NativeLibraryExtractor.extractToDefaultDir();
            if (Files.exists(libPath)) {
                return SymbolLookup.libraryLookup(libPath, Arena.global());
            }
        } catch (Exception ignored) {
            // No native library resources in JAR (non-Maven release scenario), silently skip
        }
        return null;
    }

    private static SymbolLookup lookupLibrary(Path path) {
        if (!Files.exists(path)) {
            return null;
        }
        return SymbolLookup.libraryLookup(path, Arena.global());
    }

    private static SymbolLookup loadLibraryFromSystem() {
        try {
            System.loadLibrary("sweetline");
            return SymbolLookup.loaderLookup();
        } catch (UnsatisfiedLinkError e) {
            throw new UnsatisfiedLinkError(LOAD_LIBRARY_ERROR);
        }
    }

    // Layout for sl_syntax_error_t: { int err_code; const char* err_msg; }
    static final StructLayout SYNTAX_ERROR_LAYOUT = MemoryLayout.structLayout(
            JAVA_INT.withName("err_code"),
            MemoryLayout.paddingLayout(4),
            ADDRESS.withName("err_msg")
    ).withName("sl_syntax_error_t");

    private static MethodHandle downcall(String name, FunctionDescriptor descriptor) {
        return LINKER.downcallHandle(
                LOOKUP.find(name).orElseThrow(() -> new UnsatisfiedLinkError("Symbol not found: " + name)),
                descriptor
        );
    }

    // sl_create_document(const char* uri, const char* text) -> pointer
    static final MethodHandle sl_create_document = downcall("sl_create_document",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_free_document(pointer) -> int
    static final MethodHandle sl_free_document = downcall("sl_free_document",
            FunctionDescriptor.of(JAVA_INT, ADDRESS));

    // sl_create_engine(bool show_index, bool inline_style) -> pointer
    static final MethodHandle sl_create_engine = downcall("sl_create_engine",
            FunctionDescriptor.of(ADDRESS, JAVA_BOOLEAN, JAVA_BOOLEAN));

    // sl_free_engine(pointer) -> int
    static final MethodHandle sl_free_engine = downcall("sl_free_engine",
            FunctionDescriptor.of(JAVA_INT, ADDRESS));

    // sl_engine_define_macro(pointer, const char*) -> int
    static final MethodHandle sl_engine_define_macro = downcall("sl_engine_define_macro",
            FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS));

    // sl_engine_undefine_macro(pointer, const char*) -> int
    static final MethodHandle sl_engine_undefine_macro = downcall("sl_engine_undefine_macro",
            FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS));

    // sl_engine_compile_json(pointer, const char*) -> sl_syntax_error_t (struct by value)
    static final MethodHandle sl_engine_compile_json = downcall("sl_engine_compile_json",
            FunctionDescriptor.of(SYNTAX_ERROR_LAYOUT, ADDRESS, ADDRESS));

    // sl_engine_compile_file(pointer, const char*) -> sl_syntax_error_t (struct by value)
    static final MethodHandle sl_engine_compile_file = downcall("sl_engine_compile_file",
            FunctionDescriptor.of(SYNTAX_ERROR_LAYOUT, ADDRESS, ADDRESS));

    // sl_engine_register_style_name(pointer, const char*, int) -> int
    static final MethodHandle sl_engine_register_style_name = downcall("sl_engine_register_style_name",
            FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS, JAVA_INT));

    // sl_engine_get_style_name(pointer, int) -> const char*
    static final MethodHandle sl_engine_get_style_name = downcall("sl_engine_get_style_name",
            FunctionDescriptor.of(ADDRESS, ADDRESS, JAVA_INT));

    // sl_engine_create_text_analyzer(pointer, const char*) -> pointer
    static final MethodHandle sl_engine_create_text_analyzer = downcall("sl_engine_create_text_analyzer",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_engine_create_text_analyzer_by_file_name(pointer, const char*) -> pointer
    static final MethodHandle sl_engine_create_text_analyzer_by_file_name = downcall(
            "sl_engine_create_text_analyzer_by_file_name",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_text_analyze(pointer, const char*) -> int32_t*
    static final MethodHandle sl_text_analyze = downcall("sl_text_analyze",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_text_analyze_line(pointer, const char*, int32_t*) -> int32_t*
    static final MethodHandle sl_text_analyze_line = downcall("sl_text_analyze_line",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS));

    // sl_text_analyze_indent_guides(pointer, const char*) -> int32_t*
    static final MethodHandle sl_text_analyze_indent_guides = downcall("sl_text_analyze_indent_guides",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_text_analyze_bracket_pairs(pointer, const char*) -> int32_t*
    static final MethodHandle sl_text_analyze_bracket_pairs = downcall("sl_text_analyze_bracket_pairs",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_free_text_analyzer(pointer) -> int
    static final MethodHandle sl_free_text_analyzer = downcall("sl_free_text_analyzer",
            FunctionDescriptor.of(JAVA_INT, ADDRESS));

    // sl_engine_load_document(pointer, pointer) -> pointer
    static final MethodHandle sl_engine_load_document = downcall("sl_engine_load_document",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_engine_remove_document(pointer, const char*) -> int
    static final MethodHandle sl_engine_remove_document = downcall("sl_engine_remove_document",
            FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS));

    // sl_document_analyze(pointer) -> int32_t*
    static final MethodHandle sl_document_analyze = downcall("sl_document_analyze",
            FunctionDescriptor.of(ADDRESS, ADDRESS));

    // sl_document_analyze_line_range(pointer, int32_t*) -> int32_t*
    static final MethodHandle sl_document_analyze_line_range = downcall("sl_document_analyze_line_range",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_document_analyze_incremental(pointer, int32_t*, const char*) -> int32_t*
    static final MethodHandle sl_document_analyze_incremental = downcall("sl_document_analyze_incremental",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS));

    // sl_document_analyze_incremental_in_line_range(pointer, int32_t*, const char*, int32_t*) -> int32_t*
    static final MethodHandle sl_document_analyze_incremental_in_line_range = downcall(
            "sl_document_analyze_incremental_in_line_range",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS, ADDRESS));

    // sl_document_get_highlight_slice(pointer, int32_t*) -> int32_t*
    static final MethodHandle sl_document_get_highlight_slice = downcall("sl_document_get_highlight_slice",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_document_analyze_indent_guides(pointer) -> int32_t*
    static final MethodHandle sl_document_analyze_indent_guides = downcall("sl_document_analyze_indent_guides",
            FunctionDescriptor.of(ADDRESS, ADDRESS));

    // sl_document_analyze_indent_guides_in_line_range(pointer, int32_t*) -> int32_t*
    static final MethodHandle sl_document_analyze_indent_guides_in_line_range = downcall(
            "sl_document_analyze_indent_guides_in_line_range",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_document_analyze_bracket_pairs(pointer) -> int32_t*
    static final MethodHandle sl_document_analyze_bracket_pairs = downcall("sl_document_analyze_bracket_pairs",
            FunctionDescriptor.of(ADDRESS, ADDRESS));

    // sl_document_analyze_bracket_pairs_in_line_range(pointer, int32_t*) -> int32_t*
    static final MethodHandle sl_document_analyze_bracket_pairs_in_line_range = downcall(
            "sl_document_analyze_bracket_pairs_in_line_range",
            FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS));

    // sl_free_document_analyzer(pointer) -> int
    static final MethodHandle sl_free_document_analyzer = downcall("sl_free_document_analyzer",
            FunctionDescriptor.of(JAVA_INT, ADDRESS));

    // sl_free_buffer(int32_t*) -> void
    static final MethodHandle sl_free_buffer = downcall("sl_free_buffer",
            FunctionDescriptor.ofVoid(ADDRESS));

    /**
     * Parse sl_syntax_error_t struct returned by compile functions.
     * Throws SyntaxCompileError if err_code != 0.
     */
    static void checkSyntaxError(MemorySegment errorStruct) throws SyntaxCompileError {
        int errCode = errorStruct.get(JAVA_INT, SYNTAX_ERROR_LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("err_code")));
        if (errCode != 0) {
            MemorySegment errMsgPtr = errorStruct.get(ADDRESS,
                    SYNTAX_ERROR_LAYOUT.byteOffset(MemoryLayout.PathElement.groupElement("err_msg")));
            String errMsg = "";
            if (!errMsgPtr.equals(MemorySegment.NULL)) {
                errMsg = errMsgPtr.reinterpret(Long.MAX_VALUE).getString(0);
            }
            throw new SyntaxCompileError(errCode, errMsg);
        }
    }
}
