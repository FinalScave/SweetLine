package com.qiplat.sweetline.demo

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeContentPadding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.qiplat.sweetline.Document
import com.qiplat.sweetline.BracketPairResult
import com.qiplat.sweetline.DocumentHighlight
import com.qiplat.sweetline.HighlightConfig
import com.qiplat.sweetline.HighlightEngine
import com.qiplat.sweetline.IndentGuideResult
import com.qiplat.sweetline.SyntaxCompileError
import kmp.demo.generated.resources.Res
import kotlin.time.TimeSource

private const val IMPORT_SYNTAX_NOT_FOUND = -8

@Composable
@Preview
fun App() {
    val platform = remember { getPlatform() }
    val themes = remember { HighlightTheme.builtinThemes() }
    var currentTheme by remember { mutableStateOf(themes.first()) }
    var state by remember { mutableStateOf(DemoUiState.initial()) }
    val engine = remember {
        HighlightEngine(HighlightConfig(showIndex = true, inlineStyle = false, tabSize = 4))
    }

    DisposableEffect(engine) {
        onDispose {
            engine.close()
        }
    }

    LaunchedEffect(engine) {
        state = state.copy(isLoading = true, statusText = "Preparing syntax rules...")
        runCatching {
            configureEngine(engine, platform)
            val warmup = precompileCommonSyntaxes(engine)
            state = state.copy(
                warmupSummary = "Warmup: ${warmup.compiledCount} syntaxes in ${warmup.elapsedMs} ms",
                statusText = "Compiled ${warmup.compiledCount} syntax rule files in ${warmup.elapsedMs} ms",
            )
            demoAssetFileNames.firstOrNull()?.let { fileName ->
                state = highlightFile(engine, fileName, state.warmupSummary)
            } ?: run {
                state = state.copy(isLoading = false, statusText = "No demo files available")
            }
        }.onFailure { error ->
            state = state.copy(
                isLoading = false,
                errorText = error.message ?: error::class.simpleName.orEmpty(),
                statusText = "Error: ${error.message ?: error::class.simpleName.orEmpty()}",
            )
        }
    }

    MaterialTheme {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .background(currentTheme.background),
            color = currentTheme.background,
        ) {
            Column(
                modifier = Modifier
                    .safeContentPadding()
                    .fillMaxSize()
                    .padding(14.dp)
                    .border(1.dp, currentTheme.separator, MaterialTheme.shapes.small),
            ) {
                HeaderBar(
                    theme = currentTheme,
                    platformName = platform.name,
                    selectedFile = state.selectedFile,
                    exampleFiles = demoAssetFileNames,
                    selectedTheme = currentTheme,
                    themes = themes,
                    fileSelectionEnabled = !state.isLoading,
                    onFileSelected = { fileName ->
                        state = state.copy(
                            selectedFile = fileName,
                            isLoading = true,
                            errorText = null,
                            statusText = "Loading $fileName...",
                        )
                    },
                    onThemeSelected = { currentTheme = it },
                )
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxWidth(),
                ) {
                    CodeView(
                        theme = currentTheme,
                        sourceCode = state.sourceCode,
                        highlight = state.highlight,
                        indentGuides = state.indentGuides,
                        bracketPairs = state.bracketPairs,
                        placeholder = if (state.isLoading) "Analyzing..." else state.errorText ?: "Select a file",
                        modifier = Modifier.fillMaxSize(),
                    )
                    if (state.isLoading) {
                        CircularProgressIndicator(
                            modifier = Modifier
                                .align(Alignment.TopEnd)
                                .padding(18.dp)
                                .height(20.dp),
                            strokeWidth = 2.dp,
                            color = currentTheme.accent,
                        )
                    }
                }
                StatusBar(currentTheme, state.statusText, state.errorText)
            }
        }
    }

    LaunchedEffect(state.selectedFile) {
        val selectedFile = state.selectedFile
        if (selectedFile.isNotEmpty() && state.isLoading) {
            runCatching {
                highlightFile(engine, selectedFile, state.warmupSummary)
            }.onSuccess { nextState ->
                state = nextState
            }.onFailure { error ->
                state = state.copy(
                    isLoading = false,
                    errorText = error.message ?: error::class.simpleName.orEmpty(),
                    statusText = "Error: ${error.message ?: error::class.simpleName.orEmpty()}",
                )
            }
        }
    }
}

@Composable
private fun HeaderBar(
    theme: HighlightTheme,
    platformName: String,
    selectedFile: String,
    exampleFiles: List<String>,
    selectedTheme: HighlightTheme,
    themes: List<HighlightTheme>,
    fileSelectionEnabled: Boolean,
    onFileSelected: (String) -> Unit,
    onThemeSelected: (HighlightTheme) -> Unit,
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(theme.toolbarSurface)
            .padding(horizontal = 16.dp, vertical = 14.dp),
        horizontalArrangement = Arrangement.spacedBy(14.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Column(modifier = Modifier.widthIn(min = 160.dp, max = 220.dp)) {
            Text(
                text = "SweetLine KMP Demo",
                color = theme.text,
                fontSize = 20.sp,
            )
            Text(
                text = platformName,
                color = theme.lineNumber,
                fontSize = 12.sp,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
        }
        LabeledDropdown(
            label = "File",
            value = selectedFile,
            items = exampleFiles,
            enabled = fileSelectionEnabled,
            theme = theme,
            onSelected = onFileSelected,
            modifier = Modifier.widthIn(min = 180.dp, max = 260.dp),
        )
        LabeledDropdown(
            label = "Theme",
            value = selectedTheme.name,
            items = themes.map { it.name },
            enabled = true,
            theme = theme,
            onSelected = { themeName ->
                themes.firstOrNull { it.name == themeName }?.let(onThemeSelected)
            },
            modifier = Modifier.widthIn(min = 150.dp, max = 220.dp),
        )
    }
}

@Composable
private fun LabeledDropdown(
    label: String,
    value: String,
    items: List<String>,
    enabled: Boolean,
    theme: HighlightTheme,
    onSelected: (String) -> Unit,
    modifier: Modifier = Modifier,
) {
    var expanded by remember { mutableStateOf(false) }
    Column(modifier = modifier) {
        Text(
            text = label,
            color = theme.lineNumber,
            fontSize = 11.sp,
        )
        Box {
            OutlinedButton(
                onClick = { expanded = true },
                enabled = enabled,
                colors = ButtonDefaults.outlinedButtonColors(
                    contentColor = theme.text,
                    containerColor = HighlightTheme.blend(theme.background, theme.text, 0.03f),
                ),
                border = ButtonDefaults.outlinedButtonBorder(enabled).copy(brush = SolidColor(theme.separator)),
                modifier = Modifier.fillMaxWidth(),
            ) {
                Text(
                    text = value.ifEmpty { "None" },
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    fontSize = 13.sp,
                )
            }
            DropdownMenu(
                expanded = expanded,
                onDismissRequest = { expanded = false },
                modifier = Modifier.background(theme.toolbarSurface),
            ) {
                items.forEach { item ->
                    DropdownMenuItem(
                        text = {
                            Text(
                                text = item,
                                color = theme.text,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis,
                            )
                        },
                        onClick = {
                            expanded = false
                            onSelected(item)
                        },
                    )
                }
            }
        }
    }
}

@Composable
private fun StatusBar(theme: HighlightTheme, statusText: String, errorText: String?) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .background(theme.statusSurface)
            .padding(horizontal = 16.dp, vertical = 10.dp),
    ) {
        Text(
            text = statusText,
            color = if (errorText == null) theme.lineNumber else 0xFFFF8A7Au.toInt().toColor(),
            fontSize = 12.sp,
            maxLines = 2,
            overflow = TextOverflow.Ellipsis,
        )
    }
}

private suspend fun precompileCommonSyntaxes(engine: HighlightEngine): WarmupResult {
    val jsonByResourcePath = demoCommonSyntaxResourcePaths.associateWith { path ->
        Res.readBytes(path).decodeToString()
    }
    var pending = jsonByResourcePath.keys.sorted()
    var compiledCount = 0
    val started = TimeSource.Monotonic.markNow()

    while (pending.isNotEmpty()) {
        var progressed = false
        val retryPending = mutableListOf<String>()
        pending.forEach { resourcePath ->
            try {
                engine.compileSyntaxFromJson(jsonByResourcePath.getValue(resourcePath))
                compiledCount += 1
                progressed = true
            } catch (error: SyntaxCompileError) {
                if (error.errorCode == IMPORT_SYNTAX_NOT_FOUND) {
                    retryPending += resourcePath
                } else {
                    throw SyntaxCompileError(error.errorCode, "Failed to compile ${resourcePath.substringAfterLast('/')}: ${error.message}")
                }
            }
        }
        if (!progressed) {
            throw IllegalStateException("Unresolved syntax dependencies: ${retryPending.joinToString { it.substringAfterLast('/') }}")
        }
        pending = retryPending
    }

    return WarmupResult(compiledCount, started.elapsedNow().inWholeMilliseconds)
}

private suspend fun highlightFile(
    engine: HighlightEngine,
    fileName: String,
    warmupSummary: String?,
): DemoUiState {
    val sample = demoAssetEntries.firstOrNull { it.fileName == fileName }
        ?: error("Unknown demo file: $fileName")
    val sourceCode = Res.readBytes(sample.resourcePath).decodeToString()
    val document = Document(fileName, sourceCode)
    var analyzer: com.qiplat.sweetline.DocumentAnalyzer? = null
    try {
        val loadStart = TimeSource.Monotonic.markNow()
        analyzer = engine.loadDocument(document) ?: error("No matching syntax for $fileName")
        val loadUs = loadStart.elapsedNow().inWholeMicroseconds
        val analyzeStart = TimeSource.Monotonic.markNow()
        val highlight = analyzer.analyze()
        val indentGuides = analyzer.analyzeIndentGuides()
        val bracketPairs = analyzer.analyzeBracketPairs()
        val analyzeUs = analyzeStart.elapsedNow().inWholeMicroseconds
        val lineCount = sourceCode.replace("\r\n", "\n").split('\n').size
        val statusParts = listOfNotNull(
            warmupSummary,
            "Load: ${loadUs}us",
            "Analyze: ${analyzeUs}us",
            "Lines: $lineCount",
            "File: $fileName",
        )
        return DemoUiState(
            selectedFile = fileName,
            isLoading = false,
            errorText = null,
            statusText = statusParts.joinToString(" | "),
            warmupSummary = warmupSummary,
            sourceCode = sourceCode,
            highlight = highlight,
            indentGuides = indentGuides,
            bracketPairs = bracketPairs,
        )
    } finally {
        analyzer?.close()
        document.close()
    }
}

private fun configureEngine(engine: HighlightEngine, platform: Platform) {
    platform.sweetLineMacro?.let(engine::defineMacro)
    engine.registerStyleName("keyword", HighlightTheme.STYLE_KEYWORD)
    engine.registerStyleName("string", HighlightTheme.STYLE_STRING)
    engine.registerStyleName("number", HighlightTheme.STYLE_NUMBER)
    engine.registerStyleName("comment", HighlightTheme.STYLE_COMMENT)
    engine.registerStyleName("class", HighlightTheme.STYLE_CLASS)
    engine.registerStyleName("method", HighlightTheme.STYLE_METHOD)
    engine.registerStyleName("variable", HighlightTheme.STYLE_VARIABLE)
    engine.registerStyleName("punctuation", HighlightTheme.STYLE_PUNCTUATION)
    engine.registerStyleName("annotation", HighlightTheme.STYLE_ANNOTATION)
    engine.registerStyleName("preprocessor", HighlightTheme.STYLE_PREPROCESSOR)
    engine.registerStyleName("macro", HighlightTheme.STYLE_MACRO)
    engine.registerStyleName("lifetime", HighlightTheme.STYLE_LIFETIME)
    engine.registerStyleName("selector", HighlightTheme.STYLE_SELECTOR)
    engine.registerStyleName("builtin", HighlightTheme.STYLE_BUILTIN)
    engine.registerStyleName("url", HighlightTheme.STYLE_URL)
    engine.registerStyleName("property", HighlightTheme.STYLE_PROPERTY)
}

private data class WarmupResult(
    val compiledCount: Int,
    val elapsedMs: Long,
)

private data class DemoUiState(
    val selectedFile: String,
    val isLoading: Boolean,
    val errorText: String?,
    val statusText: String,
    val warmupSummary: String?,
    val sourceCode: String,
    val highlight: DocumentHighlight?,
    val indentGuides: IndentGuideResult?,
    val bracketPairs: BracketPairResult?,
) {
    companion object {
        fun initial(): DemoUiState {
            return DemoUiState(
                selectedFile = "",
                isLoading = true,
                errorText = null,
                statusText = "Preparing syntax rules...",
                warmupSummary = null,
                sourceCode = "",
                highlight = null,
                indentGuides = null,
                bracketPairs = null,
            )
        }
    }
}
