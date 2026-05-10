import org.gradle.api.DefaultTask
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.tasks.InputDirectory
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.TaskAction
import java.io.File
import java.util.Locale

abstract class GenerateDemoAssetsTask : DefaultTask() {
    @get:InputDirectory
    abstract val syntaxesDir: DirectoryProperty

    @get:InputDirectory
    abstract val examplesDir: DirectoryProperty

    @get:OutputDirectory
    abstract val outputRoot: DirectoryProperty

    @TaskAction
    fun generate() {
        val root = outputRoot.get().asFile
        root.deleteRecursively()

        val resourcesRoot = root.resolve("composeResources")
        val generatedKotlinRoot = root.resolve("kotlin")
        val syntaxResourcesDir = resourcesRoot.resolve("files/syntaxes")
        val exampleResourcesDir = resourcesRoot.resolve("files/examples")
        val generatedPackageDir = generatedKotlinRoot.resolve("com/qiplat/sweetline/demo")
        syntaxResourcesDir.mkdirs()
        exampleResourcesDir.mkdirs()
        generatedPackageDir.mkdirs()

        val syntaxFiles = syntaxesDir.get().asFile
            .listFiles { file ->
                file.isFile &&
                    file.extension.lowercase(Locale.ROOT) == "json" &&
                    !file.name.endsWith("-inlineStyle.json") &&
                    file.name != "yaml(non zero width).json"
            }
            .orEmpty()
            .sortedBy { it.name }

        val exampleFiles = examplesDir.get().asFile
            .listFiles { file ->
                file.isFile && (file.name.startsWith("example") || file.name == "Justfile" || file.name == "meson.build")
            }
            .orEmpty()
            .sortedBy { it.name }

        syntaxFiles.forEach { source ->
            source.copyTo(syntaxResourcesDir.resolve(source.name), overwrite = true)
        }
        exampleFiles.forEach { source ->
            source.copyTo(exampleResourcesDir.resolve(source.name), overwrite = true)
        }

        val sampleEntries = buildSampleEntries(syntaxFiles, exampleFiles)
        val manifest = buildManifest(sampleEntries, syntaxFiles)
        generatedPackageDir.resolve("DemoAssets.kt").writeText(manifest)
    }

    private fun buildSampleEntries(syntaxFiles: List<File>, exampleFiles: List<File>): List<Pair<String, String>> {
        val syntaxSampleName = "json-sweetline.json"
        val entries = exampleFiles
            .map { it.name to "files/examples/${it.name}" }
            .toMutableList()
        if (syntaxFiles.any { it.name == syntaxSampleName }) {
            entries += syntaxSampleName to "files/syntaxes/$syntaxSampleName"
        }
        return entries.sortedBy { it.first }
    }

    private fun buildManifest(sampleEntries: List<Pair<String, String>>, syntaxFiles: List<File>): String {
        return buildString {
            appendLine("package com.qiplat.sweetline.demo")
            appendLine()
            appendLine("data class DemoAssetEntry(")
            appendLine("    val fileName: String,")
            appendLine("    val resourcePath: String,")
            appendLine(")")
            appendLine()
            appendLine("val demoAssetEntries: List<DemoAssetEntry> = listOf(")
            sampleEntries.forEach { (fileName, resourcePath) ->
                appendLine("    DemoAssetEntry(\"${fileName.escapeKotlinString()}\", \"${resourcePath.escapeKotlinString()}\"),")
            }
            appendLine(")")
            appendLine()
            appendLine("val demoCommonSyntaxResourcePaths: List<String> = listOf(")
            syntaxFiles.forEach { file ->
                appendLine("    \"files/syntaxes/${file.name.escapeKotlinString()}\",")
            }
            appendLine(")")
            appendLine()
            appendLine("val demoAssetFileNames: List<String> = demoAssetEntries.map { it.fileName }")
        }
    }

    private fun String.escapeKotlinString(): String {
        return replace("\\", "\\\\").replace("\"", "\\\"")
    }
}
