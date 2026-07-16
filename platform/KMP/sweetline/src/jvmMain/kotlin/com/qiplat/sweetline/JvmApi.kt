package com.qiplat.sweetline

import java.io.FileNotFoundException
import java.io.IOException
import java.lang.foreign.Arena
import java.lang.foreign.FunctionDescriptor
import java.lang.foreign.MemoryLayout
import java.lang.foreign.MemorySegment
import java.lang.foreign.StructLayout
import java.lang.foreign.SymbolLookup
import java.lang.foreign.ValueLayout.ADDRESS
import java.lang.foreign.ValueLayout.JAVA_BOOLEAN
import java.lang.foreign.ValueLayout.JAVA_INT
import java.lang.foreign.Linker
import java.lang.invoke.MethodHandle
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.StandardCopyOption

actual class HighlightEngine actual constructor(config: HighlightConfig) {
    private val handle: MemorySegment = try {
        SweetLineJvmNative.slCreateEngine.invokeWithArguments(
            config.showIndex,
            config.inlineStyle,
            config.tabSize,
        ) as MemorySegment
    } catch (error: Throwable) {
        throw RuntimeException("Failed to create SweetLine engine", error)
    }
    private var closed = false

    actual constructor() : this(HighlightConfig())

    actual fun registerStyleName(styleName: String, styleId: Int) {
        ensureOpen()
        confined { arena ->
            SweetLineJvmNative.slEngineRegisterStyleName.invokeWithArguments(handle, arena.allocateFrom(styleName), styleId)
        }
    }

    actual fun getStyleName(styleId: Int): String? {
        ensureOpen()
        val result = SweetLineJvmNative.slEngineGetStyleName.invokeWithArguments(handle, styleId) as MemorySegment
        return if (result == MemorySegment.NULL) null else result.reinterpret(Long.MAX_VALUE).getString(0)
    }

    actual fun defineMacro(macroName: String) {
        ensureOpen()
        confined { arena ->
            SweetLineJvmNative.slEngineDefineMacro.invokeWithArguments(handle, arena.allocateFrom(macroName))
        }
    }

    actual fun undefineMacro(macroName: String) {
        ensureOpen()
        confined { arena ->
            SweetLineJvmNative.slEngineUndefineMacro.invokeWithArguments(handle, arena.allocateFrom(macroName))
        }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromJson(syntaxJson: String) {
        ensureOpen()
        confined { arena ->
            val errorStruct = SweetLineJvmNative.slEngineCompileJson.invokeWithArguments(
                arena,
                handle,
                arena.allocateFrom(syntaxJson),
            ) as MemorySegment
            SweetLineJvmNative.checkSyntaxError(errorStruct)
        }
    }

    @Throws(SyntaxCompileError::class)
    actual fun compileSyntaxFromFile(path: String) {
        ensureOpen()
        confined { arena ->
            val errorStruct = SweetLineJvmNative.slEngineCompileFile.invokeWithArguments(
                arena,
                handle,
                arena.allocateFrom(path),
            ) as MemorySegment
            SweetLineJvmNative.checkSyntaxError(errorStruct)
        }
    }

    actual fun createAnalyzerBySyntaxName(syntaxName: String): TextAnalyzer? {
        ensureOpen()
        return confined { arena ->
            val analyzerHandle = SweetLineJvmNative.slEngineCreateTextAnalyzer.invokeWithArguments(
                handle,
                arena.allocateFrom(syntaxName),
            ) as MemorySegment
            analyzerHandle.takeUnless { it == MemorySegment.NULL }?.let { TextAnalyzer(it) }
        }
    }

    actual fun createAnalyzerByFileName(fileName: String): TextAnalyzer? {
        ensureOpen()
        return confined { arena ->
            val analyzerHandle = SweetLineJvmNative.slEngineCreateTextAnalyzerByFileName.invokeWithArguments(
                handle,
                arena.allocateFrom(fileName),
            ) as MemorySegment
            analyzerHandle.takeUnless { it == MemorySegment.NULL }?.let { TextAnalyzer(it) }
        }
    }

    actual fun loadDocument(document: Document): DocumentAnalyzer? {
        ensureOpen()
        val analyzerHandle = SweetLineJvmNative.slEngineLoadDocument.invokeWithArguments(handle, document.handle) as MemorySegment
        return analyzerHandle.takeUnless { it == MemorySegment.NULL }?.let { DocumentAnalyzer(it) }
    }

    actual fun removeDocument(uri: String) {
        ensureOpen()
        confined { arena ->
            val errorCode = SweetLineJvmNative.slEngineRemoveDocument.invokeWithArguments(
                handle,
                arena.allocateFrom(uri),
            ) as Int
            checkNativeError(errorCode, "remove document")
        }
    }

    actual fun close() {
        if (!closed) {
            closed = true
            SweetLineJvmNative.slFreeEngine.invokeWithArguments(handle)
        }
    }

    private fun ensureOpen() {
        check(!closed) { "HighlightEngine is already closed" }
    }
}

actual class Document actual constructor(uri: String, content: String) {
    internal val handle: MemorySegment = confined { arena ->
        SweetLineJvmNative.slCreateDocument.invokeWithArguments(
            arena.allocateFrom(uri),
            arena.allocateFrom(content),
        ) as MemorySegment
    }
    private var closed = false

    actual fun close() {
        if (!closed) {
            closed = true
            SweetLineJvmNative.slFreeDocument.invokeWithArguments(handle)
        }
    }
}

actual class TextAnalyzer internal constructor(private val handle: MemorySegment) {
    private var closed = false

    actual fun analyzeText(text: String): DocumentHighlight {
        ensureOpen()
        return confined { arena ->
            val result = SweetLineJvmNative.slTextAnalyze.invokeWithArguments(handle, arena.allocateFrom(text)) as MemorySegment
            result.readAndFree(DocumentHighlight(), ::readDocumentHighlight)
        }
    }

    actual fun analyzeLine(text: String, info: TextLineInfo): LineAnalyzeResult {
        ensureOpen()
        return confined { arena ->
            val lineInfo = arena.allocate(JAVA_INT, 3)
            lineInfo.setAtIndex(JAVA_INT, 0, info.line)
            lineInfo.setAtIndex(JAVA_INT, 1, info.startState)
            lineInfo.setAtIndex(JAVA_INT, 2, info.startCharOffset)
            val result = SweetLineJvmNative.slTextAnalyzeLine.invokeWithArguments(
                handle,
                arena.allocateFrom(text),
                lineInfo,
            ) as MemorySegment
            result.readAndFree(LineAnalyzeResult()) { segment ->
                NativeBufferParser.readLineAnalyzeResult(info.line) { index -> segment.readInt(index) }
            }
        }
    }

    actual fun analyzeIndentGuides(text: String): IndentGuideResult {
        ensureOpen()
        return confined { arena ->
            val result = SweetLineJvmNative.slTextAnalyzeIndentGuides.invokeWithArguments(
                handle,
                arena.allocateFrom(text),
            ) as MemorySegment
            result.readAndFree(IndentGuideResult(), ::readIndentGuideResult)
        }
    }

    actual fun analyzeBracketPairs(text: String): BracketPairResult {
        ensureOpen()
        return confined { arena ->
            val result = SweetLineJvmNative.slTextAnalyzeBracketPairs.invokeWithArguments(
                handle,
                arena.allocateFrom(text),
            ) as MemorySegment
            result.readAndFree(BracketPairResult(), ::readBracketPairResult)
        }
    }

    actual fun close() {
        if (!closed) {
            closed = true
            val errorCode = SweetLineJvmNative.slFreeTextAnalyzer.invokeWithArguments(handle) as Int
            checkNativeError(errorCode, "free text analyzer")
        }
    }

    private fun ensureOpen() {
        check(!closed) { "TextAnalyzer is already closed" }
    }
}

actual class DocumentAnalyzer internal constructor(private val handle: MemorySegment) {
    private var closed = false

    actual fun analyze(): DocumentHighlight {
        ensureOpen()
        val result = SweetLineJvmNative.slDocumentAnalyze.invokeWithArguments(handle) as MemorySegment
        return result.readAndFree(DocumentHighlight(), ::readDocumentHighlight)
    }

    actual fun analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice {
        ensureOpen()
        return confined { arena ->
            val visible = arena.allocate(JAVA_INT, 2)
            visible.setAtIndex(JAVA_INT, 0, visibleRange.startLine)
            visible.setAtIndex(JAVA_INT, 1, visibleRange.lineCount)
            val result = SweetLineJvmNative.slDocumentAnalyzeLineRange.invokeWithArguments(handle, visible) as MemorySegment
            result.readAndFree(DocumentHighlightSlice(), ::readDocumentHighlightSlice)
        }
    }

    actual fun analyzeIncremental(range: TextRange, newText: String): DocumentHighlight {
        ensureOpen()
        return confined { arena ->
            val changes = arena.allocate(JAVA_INT, 4)
            changes.setAtIndex(JAVA_INT, 0, range.start.line)
            changes.setAtIndex(JAVA_INT, 1, range.start.column)
            changes.setAtIndex(JAVA_INT, 2, range.end.line)
            changes.setAtIndex(JAVA_INT, 3, range.end.column)
            val result = SweetLineJvmNative.slDocumentAnalyzeIncremental.invokeWithArguments(
                handle,
                changes,
                arena.allocateFrom(newText),
            ) as MemorySegment
            result.readAndFree(DocumentHighlight(), ::readDocumentHighlight)
        }
    }

    actual fun analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange,
    ): DocumentHighlightSlice {
        ensureOpen()
        return confined { arena ->
            val changes = arena.allocate(JAVA_INT, 4)
            changes.setAtIndex(JAVA_INT, 0, range.start.line)
            changes.setAtIndex(JAVA_INT, 1, range.start.column)
            changes.setAtIndex(JAVA_INT, 2, range.end.line)
            changes.setAtIndex(JAVA_INT, 3, range.end.column)
            val visible = arena.allocate(JAVA_INT, 2)
            visible.setAtIndex(JAVA_INT, 0, visibleRange.startLine)
            visible.setAtIndex(JAVA_INT, 1, visibleRange.lineCount)
            val result = SweetLineJvmNative.slDocumentAnalyzeIncrementalInLineRange.invokeWithArguments(
                handle,
                changes,
                arena.allocateFrom(newText),
                visible,
            ) as MemorySegment
            result.readAndFree(DocumentHighlightSlice(), ::readDocumentHighlightSlice)
        }
    }

    actual fun getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice {
        ensureOpen()
        return confined { arena ->
            val visible = arena.allocate(JAVA_INT, 2)
            visible.setAtIndex(JAVA_INT, 0, visibleRange.startLine)
            visible.setAtIndex(JAVA_INT, 1, visibleRange.lineCount)
            val result = SweetLineJvmNative.slDocumentGetHighlightSlice.invokeWithArguments(handle, visible) as MemorySegment
            result.readAndFree(DocumentHighlightSlice(), ::readDocumentHighlightSlice)
        }
    }

    actual fun analyzeIndentGuides(): IndentGuideResult {
        ensureOpen()
        val result = SweetLineJvmNative.slDocumentAnalyzeIndentGuides.invokeWithArguments(handle) as MemorySegment
        return result.readAndFree(IndentGuideResult(), ::readIndentGuideResult)
    }

    actual fun analyzeIndentGuidesInLineRange(visibleRange: LineRange): IndentGuideResult {
        ensureOpen()
        return confined { arena ->
            val visible = arena.allocate(JAVA_INT, 2)
            visible.setAtIndex(JAVA_INT, 0, visibleRange.startLine)
            visible.setAtIndex(JAVA_INT, 1, visibleRange.lineCount)
            val result = SweetLineJvmNative.slDocumentAnalyzeIndentGuidesInLineRange
                .invokeWithArguments(handle, visible) as MemorySegment
            result.readAndFree(IndentGuideResult(), ::readIndentGuideResult)
        }
    }

    actual fun analyzeBracketPairs(): BracketPairResult {
        ensureOpen()
        val result = SweetLineJvmNative.slDocumentAnalyzeBracketPairs.invokeWithArguments(handle) as MemorySegment
        return result.readAndFree(BracketPairResult(), ::readBracketPairResult)
    }

    actual fun analyzeBracketPairsInLineRange(visibleRange: LineRange): BracketPairResult {
        ensureOpen()
        return confined { arena ->
            val visible = arena.allocate(JAVA_INT, 2)
            visible.setAtIndex(JAVA_INT, 0, visibleRange.startLine)
            visible.setAtIndex(JAVA_INT, 1, visibleRange.lineCount)
            val result = SweetLineJvmNative.slDocumentAnalyzeBracketPairsInLineRange
                .invokeWithArguments(handle, visible) as MemorySegment
            result.readAndFree(BracketPairResult(), ::readBracketPairResultSlice)
        }
    }

    actual fun close() {
        if (!closed) {
            closed = true
            val errorCode = SweetLineJvmNative.slFreeDocumentAnalyzer.invokeWithArguments(handle) as Int
            checkNativeError(errorCode, "free document analyzer")
        }
    }

    private fun ensureOpen() {
        check(!closed) { "DocumentAnalyzer is already closed" }
    }
}

private object SweetLineJvmNative {
    private const val LIB_PATH_KEY = "sweetline.lib.path"
    private const val NATIVE_RESOURCE_ROOT = "/native/"
    private const val LOAD_LIBRARY_ERROR =
        "Cannot load native library 'sweetline'. Set -Dsweetline.lib.path=<dir> or add the library to java.library.path."

    private val linker: Linker = Linker.nativeLinker()
    private val lookup: SymbolLookup = loadLibraryLookup()

    private val syntaxErrorLayout: StructLayout = MemoryLayout.structLayout(
        JAVA_INT.withName("err_code"),
        MemoryLayout.paddingLayout(4),
        ADDRESS.withName("err_msg"),
    ).withName("sl_syntax_error_t")

    val slCreateDocument: MethodHandle = downcall(
        "sl_create_document",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slFreeDocument: MethodHandle = downcall(
        "sl_free_document",
        FunctionDescriptor.of(JAVA_INT, ADDRESS),
    )
    val slCreateEngine: MethodHandle = downcall(
        "sl_create_engine",
        FunctionDescriptor.of(ADDRESS, JAVA_BOOLEAN, JAVA_BOOLEAN, JAVA_INT),
    )
    val slFreeEngine: MethodHandle = downcall(
        "sl_free_engine",
        FunctionDescriptor.of(JAVA_INT, ADDRESS),
    )
    val slEngineDefineMacro: MethodHandle = downcall(
        "sl_engine_define_macro",
        FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS),
    )
    val slEngineUndefineMacro: MethodHandle = downcall(
        "sl_engine_undefine_macro",
        FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS),
    )
    val slEngineCompileJson: MethodHandle = downcall(
        "sl_engine_compile_json",
        FunctionDescriptor.of(syntaxErrorLayout, ADDRESS, ADDRESS),
    )
    val slEngineCompileFile: MethodHandle = downcall(
        "sl_engine_compile_file",
        FunctionDescriptor.of(syntaxErrorLayout, ADDRESS, ADDRESS),
    )
    val slEngineRegisterStyleName: MethodHandle = downcall(
        "sl_engine_register_style_name",
        FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS, JAVA_INT),
    )
    val slEngineGetStyleName: MethodHandle = downcall(
        "sl_engine_get_style_name",
        FunctionDescriptor.of(ADDRESS, ADDRESS, JAVA_INT),
    )
    val slEngineCreateTextAnalyzer: MethodHandle = downcall(
        "sl_engine_create_text_analyzer",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slEngineCreateTextAnalyzerByFileName: MethodHandle = downcall(
        "sl_engine_create_text_analyzer_by_file_name",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slTextAnalyze: MethodHandle = downcall(
        "sl_text_analyze",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slTextAnalyzeLine: MethodHandle = downcall(
        "sl_text_analyze_line",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS),
    )
    val slTextAnalyzeIndentGuides: MethodHandle = downcall(
        "sl_text_analyze_indent_guides",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slTextAnalyzeBracketPairs: MethodHandle = downcall(
        "sl_text_analyze_bracket_pairs",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slFreeTextAnalyzer: MethodHandle = downcall(
        "sl_free_text_analyzer",
        FunctionDescriptor.of(JAVA_INT, ADDRESS),
    )
    val slEngineLoadDocument: MethodHandle = downcall(
        "sl_engine_load_document",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slEngineRemoveDocument: MethodHandle = downcall(
        "sl_engine_remove_document",
        FunctionDescriptor.of(JAVA_INT, ADDRESS, ADDRESS),
    )
    val slDocumentAnalyze: MethodHandle = downcall(
        "sl_document_analyze",
        FunctionDescriptor.of(ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeLineRange: MethodHandle = downcall(
        "sl_document_analyze_line_range",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeIncremental: MethodHandle = downcall(
        "sl_document_analyze_incremental",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeIncrementalInLineRange: MethodHandle = downcall(
        "sl_document_analyze_incremental_in_line_range",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS, ADDRESS, ADDRESS),
    )
    val slDocumentGetHighlightSlice: MethodHandle = downcall(
        "sl_document_get_highlight_slice",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeIndentGuides: MethodHandle = downcall(
        "sl_document_analyze_indent_guides",
        FunctionDescriptor.of(ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeIndentGuidesInLineRange: MethodHandle = downcall(
        "sl_document_analyze_indent_guides_in_line_range",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeBracketPairs: MethodHandle = downcall(
        "sl_document_analyze_bracket_pairs",
        FunctionDescriptor.of(ADDRESS, ADDRESS),
    )
    val slDocumentAnalyzeBracketPairsInLineRange: MethodHandle = downcall(
        "sl_document_analyze_bracket_pairs_in_line_range",
        FunctionDescriptor.of(ADDRESS, ADDRESS, ADDRESS),
    )
    val slFreeDocumentAnalyzer: MethodHandle = downcall(
        "sl_free_document_analyzer",
        FunctionDescriptor.of(JAVA_INT, ADDRESS),
    )
    val slFreeBuffer: MethodHandle = downcall(
        "sl_free_buffer",
        FunctionDescriptor.ofVoid(ADDRESS),
    )

    fun checkSyntaxError(errorStruct: MemorySegment) {
        val errCode = errorStruct.get(
            JAVA_INT,
            syntaxErrorLayout.byteOffset(MemoryLayout.PathElement.groupElement("err_code")),
        )
        if (errCode != 0) {
            val errMsgPtr = errorStruct.get(
                ADDRESS,
                syntaxErrorLayout.byteOffset(MemoryLayout.PathElement.groupElement("err_msg")),
            )
            val errMsg = if (errMsgPtr == MemorySegment.NULL) "" else errMsgPtr.reinterpret(Long.MAX_VALUE).getString(0)
            throw SyntaxCompileError(errCode, errMsg)
        }
    }

    private fun downcall(name: String, descriptor: FunctionDescriptor): MethodHandle {
        val symbol = lookup.find(name).orElseThrow { UnsatisfiedLinkError("Symbol not found: $name") }
        return linker.downcallHandle(symbol, descriptor)
    }

    private fun loadLibraryLookup(): SymbolLookup {
        val libName = System.mapLibraryName("sweetline")

        tryExplicitLibrary(libName)?.let { return it }
        tryLoadFromJarResources()?.let { return it }

        return loadLibraryFromSystem()
    }

    private fun tryExplicitLibrary(libName: String): SymbolLookup? {
        val libPath = System.getProperty(LIB_PATH_KEY)
        if (libPath.isNullOrBlank()) {
            return null
        }
        return lookupLibrary(Path.of(libPath, libName))
    }

    private fun tryLoadFromJarResources(): SymbolLookup? {
        return try {
            val libPath = NativeJvmLibraryExtractor.extractToDefaultDir()
            if (Files.exists(libPath)) {
                SymbolLookup.libraryLookup(libPath, Arena.global())
            } else {
                null
            }
        } catch (_: Exception) {
            null
        }
    }

    private fun lookupLibrary(path: Path): SymbolLookup? {
        return if (Files.exists(path)) SymbolLookup.libraryLookup(path, Arena.global()) else null
    }

    private fun loadLibraryFromSystem(): SymbolLookup {
        try {
            System.loadLibrary("sweetline")
            return SymbolLookup.loaderLookup()
        } catch (_: UnsatisfiedLinkError) {
            throw UnsatisfiedLinkError(LOAD_LIBRARY_ERROR)
        }
    }

    private object NativeJvmLibraryExtractor {
        fun extractToDefaultDir(): Path {
            return extract(defaultNativeDir())
        }

        private fun extract(targetDir: Path): Path {
            val libName = System.mapLibraryName("sweetline")
            val platform = detectPlatform()
            val resourcePath = "$NATIVE_RESOURCE_ROOT$platform/$libName"

            Files.createDirectories(targetDir)
            val targetFile = targetDir.resolve(libName)
            if (!needsExtraction(targetFile, resourcePath)) {
                registerLibraryPath(targetDir)
                return targetFile
            }

            val input = NativeJvmLibraryExtractor::class.java.getResourceAsStream(resourcePath)
                ?: throw FileNotFoundException("Native library not found in resources: $resourcePath")
            input.use {
                Files.copy(it, targetFile, StandardCopyOption.REPLACE_EXISTING)
            }
            registerLibraryPath(targetDir)
            return targetFile
        }

        private fun defaultNativeDir(): Path {
            val os = System.getProperty("os.name", "").lowercase()
            val userHome = System.getProperty("user.home")
            return when {
                os.contains("win") -> {
                    val localAppData = System.getenv("LOCALAPPDATA")
                    if (!localAppData.isNullOrEmpty()) {
                        Path.of(localAppData, "SweetLine", "native")
                    } else {
                        Path.of(userHome, "AppData", "Local", "SweetLine", "native")
                    }
                }
                os.contains("mac") || os.contains("darwin") -> {
                    Path.of(userHome, "Library", "Application Support", "SweetLine", "native")
                }
                else -> {
                    val xdgDataHome = System.getenv("XDG_DATA_HOME")
                    if (!xdgDataHome.isNullOrEmpty()) {
                        Path.of(xdgDataHome, "sweetline", "native")
                    } else {
                        Path.of(userHome, ".local", "share", "sweetline", "native")
                    }
                }
            }
        }

        private fun needsExtraction(targetFile: Path, resourcePath: String): Boolean {
            if (!Files.exists(targetFile)) {
                return true
            }
            val resourceSize = resourceSize(resourcePath)
            if (resourceSize < 0) {
                return true
            }
            return Files.size(targetFile) != resourceSize
        }

        private fun resourceSize(resourcePath: String): Long {
            return try {
                NativeJvmLibraryExtractor::class.java.getResourceAsStream(resourcePath)?.use {
                    it.readAllBytes().size.toLong()
                } ?: -1
            } catch (_: IOException) {
                -1
            }
        }

        private fun registerLibraryPath(targetDir: Path) {
            System.setProperty(LIB_PATH_KEY, targetDir.toAbsolutePath().toString())
        }

        private fun detectPlatform(): String {
            val os = System.getProperty("os.name", "").lowercase()
            val arch = System.getProperty("os.arch", "").lowercase()
            val osName = when {
                os.contains("win") -> "windows"
                os.contains("mac") || os.contains("darwin") -> "macos"
                else -> "linux"
            }
            val archName = when {
                arch.contains("aarch64") || arch.contains("arm64") -> "aarch64"
                arch.contains("amd64") || arch.contains("x86_64") || arch.contains("x64") -> "x86_64"
                else -> arch
            }
            return "$osName-$archName"
        }
    }
}

private fun checkNativeError(errorCode: Int, action: String) {
    check(errorCode == 0) { "Failed to $action. Native error code: $errorCode" }
}

private inline fun <T> confined(block: (Arena) -> T): T {
    val arena = Arena.ofConfined()
    return try {
        block(arena)
    } finally {
        arena.close()
    }
}

private fun readDocumentHighlight(segment: MemorySegment): DocumentHighlight {
    return NativeBufferParser.readDocumentHighlight { index -> segment.readInt(index) }
}

private fun readDocumentHighlightSlice(segment: MemorySegment): DocumentHighlightSlice {
    return NativeBufferParser.readDocumentHighlightSlice { index -> segment.readInt(index) }
}

private fun readIndentGuideResult(segment: MemorySegment): IndentGuideResult {
    return NativeBufferParser.readIndentGuideResult { index -> segment.readInt(index) }
}

private fun readBracketPairResult(segment: MemorySegment): BracketPairResult {
    return NativeBufferParser.readBracketPairResult { index -> segment.readInt(index) }
}

private fun readBracketPairResultSlice(segment: MemorySegment): BracketPairResult {
    return NativeBufferParser.readBracketPairResultSlice { index -> segment.readInt(index) }
}

private fun <T> MemorySegment.readAndFree(defaultValue: T, reader: (MemorySegment) -> T): T {
    if (this == MemorySegment.NULL) {
        return defaultValue
    }
    return try {
        reader(this)
    } finally {
        SweetLineJvmNative.slFreeBuffer.invokeWithArguments(this)
    }
}

private fun MemorySegment.readInt(index: Int): Int {
    return reinterpret(Long.MAX_VALUE).getAtIndex(JAVA_INT, index.toLong())
}
