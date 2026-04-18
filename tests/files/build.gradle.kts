plugins {
    kotlin("jvm") version "2.0.21"
    application
}

group = "com.sweetline.demo"
version = "1.2.2"
description = "SweetLine Gradle Kotlin DSL example"

repositories {
    mavenCentral()
    google()
    gradlePluginPortal()
}

dependencies {
    implementation(kotlin("stdlib"))
    implementation("com.google.guava:guava:33.4.8-jre")
    testImplementation(platform("org.junit:junit-bom:5.11.4"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testRuntimeOnly("org.junit.platform:junit-platform-launcher")
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(21))
    }
}

application {
    mainClass.set("com.sweetline.demo.MainKt")
}

tasks.withType<Test>().configureEach {
    useJUnitPlatform()
}

tasks.register<Copy>("copyDocs") {
    from("docs")
    into(layout.buildDirectory.dir("docs"))
}

publishing {
    publications {
        create<MavenPublication>("mavenJava") {
            from(components["java"])
            artifactId = "sweetline-demo"
        }
    }
}

android {
    namespace = "com.sweetline.demo"
    compileSdk = 35

    defaultConfig {
        minSdk = 24
        targetSdk = 35
        versionCode = 122
        versionName = "1.2.2"
    }
}

data class Sample(val name: String, val enabled: Boolean)

fun greeting(name: String, repeatCount: Int = 1): String {
    return buildString {
        repeat(repeatCount) {
            appendLine("Hello, $name")
        }
    }
}
