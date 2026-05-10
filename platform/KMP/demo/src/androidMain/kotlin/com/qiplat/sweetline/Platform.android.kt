package com.qiplat.sweetline.demo

import android.os.Build

class AndroidPlatform : Platform {
    override val name: String = "Android ${Build.VERSION.SDK_INT}"
    override val sweetLineMacro: String = "ANDROID"
}

actual fun getPlatform(): Platform = AndroidPlatform()
