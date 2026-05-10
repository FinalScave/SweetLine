import org.jetbrains.compose.desktop.application.dsl.TargetFormat
import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    alias(libs.plugins.kotlinMultiplatform)
    alias(libs.plugins.androidApplication)
    alias(libs.plugins.composeMultiplatform)
    alias(libs.plugins.composeCompiler)
    alias(libs.plugins.composeHotReload)
}

val repoRoot = rootProject.projectDir.resolve("../..").normalize()
val generatedDemoAssetsRoot = layout.buildDirectory.dir("generated/demoAssets")
val generatedDemoComposeResourcesDir = generatedDemoAssetsRoot.map { it.dir("composeResources") }
val generatedDemoKotlinDir = generatedDemoAssetsRoot.map { it.dir("kotlin") }

val generateDemoAssets by tasks.registering(GenerateDemoAssetsTask::class) {
    syntaxesDir.set(repoRoot.resolve("syntaxes"))
    examplesDir.set(repoRoot.resolve("tests/files"))
    outputRoot.set(generatedDemoAssetsRoot)
}

kotlin {
    jvmToolchain(22)

    androidTarget {
        compilerOptions {
            jvmTarget.set(JvmTarget.JVM_11)
        }
    }

    listOf(
        iosArm64(),
        iosSimulatorArm64()
    ).forEach { iosTarget ->
        iosTarget.binaries.framework {
            baseName = "SweetLineDemo"
            isStatic = true
        }
    }

    jvm {
        compilerOptions {
            jvmTarget.set(JvmTarget.JVM_22)
        }
    }

    sourceSets {
        androidMain.dependencies {
            implementation(projects.sweetline)
            implementation(libs.compose.uiToolingPreview)
            implementation(libs.androidx.activity.compose)
        }
        commonMain {
            kotlin.srcDir(generatedDemoKotlinDir)
            dependencies {
                implementation(projects.sweetline)
                implementation(libs.compose.runtime)
                implementation(libs.compose.foundation)
                implementation(libs.compose.material3)
                implementation(libs.compose.ui)
                implementation(libs.compose.components.resources)
                implementation(libs.compose.uiToolingPreview)
                implementation(libs.androidx.lifecycle.viewmodelCompose)
                implementation(libs.androidx.lifecycle.runtimeCompose)
            }
        }
        commonTest.dependencies {
            implementation(libs.kotlin.test)
        }
        jvmMain.dependencies {
            implementation(projects.sweetline)
            implementation(compose.desktop.currentOs)
            implementation(libs.kotlinx.coroutinesSwing)
        }
    }
}

compose.resources {
    customDirectory(
        sourceSetName = "commonMain",
        directoryProvider = generatedDemoComposeResourcesDir,
    )
}

android {
    namespace = "com.qiplat.sweetline.demo"
    compileSdk = libs.versions.android.compileSdk.get().toInt()

    defaultConfig {
        applicationId = "com.qiplat.sweetline.demo"
        minSdk = libs.versions.android.minSdk.get().toInt()
        targetSdk = libs.versions.android.targetSdk.get().toInt()
        versionCode = 1
        versionName = "1.0"
    }
    packaging {
        resources {
            excludes += "/META-INF/{AL2.0,LGPL2.1}"
        }
    }
    buildTypes {
        getByName("release") {
            isMinifyEnabled = false
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    debugImplementation(libs.compose.uiTooling)
}

val java22Launcher = javaToolchains.launcherFor {
    languageVersion.set(JavaLanguageVersion.of(22))
}

tasks.withType<JavaExec>().configureEach {
    javaLauncher.set(java22Launcher)
}

tasks.configureEach {
    if (name != generateDemoAssets.name &&
        (name.startsWith("compile") ||
            name.startsWith("convertXmlValueResources") ||
            name.startsWith("copyNonXmlValueResources") ||
            name.startsWith("generateResource") ||
            name.startsWith("prepareComposeResources") ||
            name.contains("ComposeResources"))
    ) {
        dependsOn(generateDemoAssets)
    }
}

compose.desktop {
    application {
        mainClass = "com.qiplat.sweetline.demo.MainKt"

        nativeDistributions {
            targetFormats(TargetFormat.Dmg, TargetFormat.Msi, TargetFormat.Deb)
            packageName = "com.qiplat.sweetline.demo"
            packageVersion = "1.0.0"
        }
    }
}
