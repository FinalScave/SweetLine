package com.qiplat.sweetline

import kotlin.test.Test

class SweetLineJvmSmokeTest {
    @Test
    fun createsEngine() {
        val engine = HighlightEngine()
        engine.close()
    }
}
