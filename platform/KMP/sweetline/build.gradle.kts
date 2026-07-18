import org.gradle.plugins.signing.SigningExtension
import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    alias(libs.plugins.kotlinMultiplatform)
    alias(libs.plugins.androidLibrary)
    alias(libs.plugins.mavenPublish)
}

description = "Kotlin Multiplatform wrapper for the SweetLine native syntax highlighting engine."
group = "com.qiplat"
version = "1.0.0"

val repoRoot = rootProject.projectDir.resolve("../..").normalize()
val generatedJvmNativeResourcesDir = layout.buildDirectory.dir("generated/resources/jvm/main")

plugins.withId("signing") {
    extensions.configure<SigningExtension>("signing") {
        useGpgCmd()
    }
}

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

mavenPublishing {
    publishToMavenCentral()
    signAllPublications()
    coordinates(group.toString(), "sweetline-kmp", version.toString())

    pom {
        name.set("SweetLine KMP")
        description.set(project.description)
        url.set("https://github.com/FinalScave/SweetLine")
        licenses {
            license {
                name.set("MIT License")
                url.set("https://opensource.org/license/mit")
            }
        }
        developers {
            developer {
                name.set("Scave")
                email.set("iscave@163.com")
                organization.set("Mobile IPE")
                organizationUrl.set("https://github.com/mobile-ipe")
            }
        }
        scm {
            connection.set("scm:git:https://github.com/FinalScave/SweetLine.git")
            developerConnection.set("scm:git:https://github.com/FinalScave/SweetLine")
            url.set("https://github.com/FinalScave/SweetLine")
        }
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
    from(repoRoot.resolve("prebuilt/macos/x86_64")) {
        include("libsweetline.dylib")
        into("native/macos-x86_64")
    }
    from(repoRoot.resolve("prebuilt/macos/arm64")) {
        include("libsweetline.dylib")
        into("native/macos-aarch64")
    }
}

tasks.matching { it.name == "jvmProcessResources" }.configureEach {
    dependsOn(syncJvmNativeResources)
}
