package com.qiplat.sweetline.demo

class JVMPlatform : Platform {
    override val name: String = "Java ${System.getProperty("java.version")}"
    override val sweetLineMacro: String? = when {
        System.getProperty("os.name", "").contains("win", ignoreCase = true) -> "WINDOWS"
        System.getProperty("os.name", "").contains("mac", ignoreCase = true) -> "MACOS"
        System.getProperty("os.name", "").contains("linux", ignoreCase = true) -> "LINUX"
        else -> null
    }
}

actual fun getPlatform(): Platform = JVMPlatform()
