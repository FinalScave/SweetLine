import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    alias(libs.plugins.kotlinMultiplatform)
    alias(libs.plugins.androidLibrary)
}

group = "com.qiplat"
version = "1.2.5"

val repoRoot = rootProject.projectDir.resolve("../..").normalize()
val generatedJvmNativeResourcesDir = layout.buildDirectory.dir("generated/resources/jvm/main")

kotlin {
    jvmToolchain(22)

    compilerOptions {
        freeCompilerArgs.add("-Xexpect-actual-classes")
    }

    androidTarget {
        compilerOptions {
            jvmTarget.set(JvmTarget.JVM_11)
        }
    }

    jvm {
        compilerOptions {
            jvmTarget.set(JvmTarget.JVM_22)
        }
    }

    listOf(
        iosArm64(),
        iosSimulatorArm64(),
    ).forEach { iosTarget ->
        iosTarget.compilations.getByName("main") {
            cinterops {
                val sweetline by creating {
                    defFile(project.file("src/nativeInterop/cinterop/sweetline.def"))
                    includeDirs(repoRoot.resolve("include"))
                }
            }
        }
        iosTarget.binaries.all {
            val archDir = when (iosTarget.name) {
                "iosArm64" -> "arm64"
                else -> "simulator-arm64"
            }
            linkerOpts("-L${repoRoot.resolve("prebuilt/ios/$archDir").absolutePath}", "-lsweetline")
        }
    }

    sourceSets {
        commonTest.dependencies {
            implementation(libs.kotlin.test)
        }
        jvmMain {
            resources.srcDir(generatedJvmNativeResourcesDir)
        }
    }
}

android {
    namespace = "com.qiplat.sweetline"
    compileSdk = libs.versions.android.compileSdk.get().toInt()
    ndkVersion = "28.2.13676358"

    defaultConfig {
        minSdk = libs.versions.android.minSdk.get().toInt()
        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_static",
                    "-DANDROID=ON",
                    "-DSWEETLINE_BUILD_SHARED=ON",
                    "-DSWEETLINE_BUILD_STATIC=OFF",
                    "-DSWEETLINE_BUILD_TESTS=OFF",
                )
            }
        }
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
        consumerProguardFiles("consumer-rules.pro")
    }

    externalNativeBuild {
        cmake {
            path = repoRoot.resolve("CMakeLists.txt")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

val syncJvmNativeResources by tasks.registering(Sync::class) {
    into(generatedJvmNativeResourcesDir)

    from(repoRoot.resolve("prebuilt/windows/x64")) {
        include("sweetline.dll")
        into("native/windows-x86_64")
    }
    from(repoRoot.resolve("prebuilt/linux/x86_64")) {
        include("libsweetline.so")
        into("native/linux-x86_64")
    }
    from(repoRoot.resolve("prebuilt/linux/aarch64")) {
        include("libsweetline.so")
        into("native/linux-aarch64")
    }
    from(repoRoot.resolve("prebuilt/osx/x86_64")) {
        include("libsweetline.dylib")
        into("native/macos-x86_64")
    }
    from(repoRoot.resolve("prebuilt/osx/arm64")) {
        include("libsweetline.dylib")
        into("native/macos-aarch64")
    }
}

tasks.matching { it.name == "jvmProcessResources" }.configureEach {
    dependsOn(syncJvmNativeResources)
}
