package com.qiplat.sweetline.demo

import platform.UIKit.UIDevice

class IOSPlatform : Platform {
    override val name: String = UIDevice.currentDevice.systemName() + " " + UIDevice.currentDevice.systemVersion
    override val sweetLineMacro: String = "IOS"
}

actual fun getPlatform(): Platform = IOSPlatform()
