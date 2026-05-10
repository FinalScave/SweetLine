package com.qiplat.sweetline.demo

interface Platform {
    val name: String
    val sweetLineMacro: String?
}

expect fun getPlatform(): Platform
