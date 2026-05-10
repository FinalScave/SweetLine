package com.qiplat.sweetline.demo

import androidx.compose.ui.graphics.Color

data class HighlightTheme(
    val name: String,
    val backgroundColor: Int,
    val textColor: Int,
    private val colorMap: Map<Int, Int>,
) {
    val background: Color = backgroundColor.toColor()
    val text: Color = textColor.toColor()
    val gutterBackground: Color = blend(background, text, 0.08f)
    val separator: Color = blend(background, text, 0.14f)
    val lineNumber: Color = blend(text, background, 0.48f)
    val indentGuide: Color = blend(text, background, 0.34f)
    val toolbarSurface: Color = blend(background, Color.White, 0.035f)
    val statusSurface: Color = blend(background, Color.White, 0.02f)
    val accent: Color = getColor(STYLE_KEYWORD).toColor()

    fun getColor(styleId: Int): Int {
        return colorMap[styleId] ?: textColor
    }

    companion object {
        const val STYLE_KEYWORD: Int = 1
        const val STYLE_STRING: Int = 2
        const val STYLE_NUMBER: Int = 3
        const val STYLE_COMMENT: Int = 4
        const val STYLE_CLASS: Int = 5
        const val STYLE_METHOD: Int = 6
        const val STYLE_VARIABLE: Int = 7
        const val STYLE_PUNCTUATION: Int = 8
        const val STYLE_ANNOTATION: Int = 9
        const val STYLE_PREPROCESSOR: Int = 10
        const val STYLE_MACRO: Int = 11
        const val STYLE_LIFETIME: Int = 12
        const val STYLE_SELECTOR: Int = 13
        const val STYLE_BUILTIN: Int = 14
        const val STYLE_URL: Int = 15
        const val STYLE_PROPERTY: Int = 16

        fun builtinThemes(): List<HighlightTheme> {
            return listOf(
                sweetLineDark(),
                monokai(),
                dracula(),
                oneDark(),
                solarizedDark(),
                nord(),
                githubDark(),
            )
        }

        fun blend(base: Color, target: Color, ratio: Float): Color {
            val clamped = ratio.coerceIn(0f, 1f)
            return Color(
                red = base.red * (1f - clamped) + target.red * clamped,
                green = base.green * (1f - clamped) + target.green * clamped,
                blue = base.blue * (1f - clamped) + target.blue * clamped,
                alpha = base.alpha * (1f - clamped) + target.alpha * clamped,
            )
        }

        private fun sweetLineDark(): HighlightTheme {
            return HighlightTheme("SweetLine Dark", 0xFF1E1E1Eu.toInt(), 0xFFD4D4D4u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFF569CD6u.toInt(),
                STYLE_STRING to 0xFFBD63C5u.toInt(),
                STYLE_NUMBER to 0xFFE4FAD5u.toInt(),
                STYLE_COMMENT to 0xFF60AE6Fu.toInt(),
                STYLE_CLASS to 0xFF4EC9B0u.toInt(),
                STYLE_METHOD to 0xFF9CDCFEu.toInt(),
                STYLE_VARIABLE to 0xFF9B9BC8u.toInt(),
                STYLE_PUNCTUATION to 0xFFD69D85u.toInt(),
                STYLE_ANNOTATION to 0xFFFFFD9Bu.toInt(),
                STYLE_PREPROCESSOR to 0xFF569CD6u.toInt(),
                STYLE_MACRO to 0xFF9B9BC8u.toInt(),
                STYLE_LIFETIME to 0xFF4EC9B0u.toInt(),
                STYLE_SELECTOR to 0xFF4EC9B0u.toInt(),
                STYLE_BUILTIN to 0xFF569CD6u.toInt(),
                STYLE_URL to 0xFF4FC1FFu.toInt(),
                STYLE_PROPERTY to 0xFF9CDCFEu.toInt(),
            ))
        }

        private fun monokai(): HighlightTheme {
            return HighlightTheme("Monokai", 0xFF272822u.toInt(), 0xFFF8F8F2u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFFF92672u.toInt(),
                STYLE_STRING to 0xFFE6DB74u.toInt(),
                STYLE_NUMBER to 0xFFAE81FFu.toInt(),
                STYLE_COMMENT to 0xFF75715Eu.toInt(),
                STYLE_CLASS to 0xFFA6E22Eu.toInt(),
                STYLE_METHOD to 0xFFA6E22Eu.toInt(),
                STYLE_VARIABLE to 0xFFF8F8F2u.toInt(),
                STYLE_PUNCTUATION to 0xFFF8F8F2u.toInt(),
                STYLE_ANNOTATION to 0xFFE6DB74u.toInt(),
                STYLE_PREPROCESSOR to 0xFFF92672u.toInt(),
                STYLE_MACRO to 0xFFAE81FFu.toInt(),
                STYLE_LIFETIME to 0xFFFD971Fu.toInt(),
                STYLE_SELECTOR to 0xFFA6E22Eu.toInt(),
                STYLE_BUILTIN to 0xFF66D9EFu.toInt(),
                STYLE_URL to 0xFF66D9EFu.toInt(),
                STYLE_PROPERTY to 0xFFA6E22Eu.toInt(),
            ))
        }

        private fun dracula(): HighlightTheme {
            return HighlightTheme("Dracula", 0xFF282A36u.toInt(), 0xFFF8F8F2u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFFFF79C6u.toInt(),
                STYLE_STRING to 0xFFF1FA8Cu.toInt(),
                STYLE_NUMBER to 0xFFBD93F9u.toInt(),
                STYLE_COMMENT to 0xFF6272A4u.toInt(),
                STYLE_CLASS to 0xFF8BE9FDu.toInt(),
                STYLE_METHOD to 0xFF50FA7Bu.toInt(),
                STYLE_VARIABLE to 0xFFF8F8F2u.toInt(),
                STYLE_PUNCTUATION to 0xFFF8F8F2u.toInt(),
                STYLE_ANNOTATION to 0xFFFFB86Cu.toInt(),
                STYLE_PREPROCESSOR to 0xFFFF79C6u.toInt(),
                STYLE_MACRO to 0xFFBD93F9u.toInt(),
                STYLE_LIFETIME to 0xFFFFB86Cu.toInt(),
                STYLE_SELECTOR to 0xFF50FA7Bu.toInt(),
                STYLE_BUILTIN to 0xFF8BE9FDu.toInt(),
                STYLE_URL to 0xFF8BE9FDu.toInt(),
                STYLE_PROPERTY to 0xFF50FA7Bu.toInt(),
            ))
        }

        private fun oneDark(): HighlightTheme {
            return HighlightTheme("One Dark", 0xFF282C34u.toInt(), 0xFFABB2BFu.toInt(), mapOf(
                STYLE_KEYWORD to 0xFFC678DDu.toInt(),
                STYLE_STRING to 0xFF98C379u.toInt(),
                STYLE_NUMBER to 0xFFD19A66u.toInt(),
                STYLE_COMMENT to 0xFF5C6370u.toInt(),
                STYLE_CLASS to 0xFFE5C07Bu.toInt(),
                STYLE_METHOD to 0xFF61AFEFu.toInt(),
                STYLE_VARIABLE to 0xFFE06C75u.toInt(),
                STYLE_PUNCTUATION to 0xFFABB2BFu.toInt(),
                STYLE_ANNOTATION to 0xFFE5C07Bu.toInt(),
                STYLE_PREPROCESSOR to 0xFFC678DDu.toInt(),
                STYLE_MACRO to 0xFFD19A66u.toInt(),
                STYLE_LIFETIME to 0xFF56B6C2u.toInt(),
                STYLE_SELECTOR to 0xFFE5C07Bu.toInt(),
                STYLE_BUILTIN to 0xFF56B6C2u.toInt(),
                STYLE_URL to 0xFF61AFEFu.toInt(),
                STYLE_PROPERTY to 0xFF61AFEFu.toInt(),
            ))
        }

        private fun solarizedDark(): HighlightTheme {
            return HighlightTheme("Solarized Dark", 0xFF002B36u.toInt(), 0xFF839496u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFF859900u.toInt(),
                STYLE_STRING to 0xFF2AA198u.toInt(),
                STYLE_NUMBER to 0xFFD33682u.toInt(),
                STYLE_COMMENT to 0xFF586E75u.toInt(),
                STYLE_CLASS to 0xFFB58900u.toInt(),
                STYLE_METHOD to 0xFF268BD2u.toInt(),
                STYLE_VARIABLE to 0xFFCB4B16u.toInt(),
                STYLE_PUNCTUATION to 0xFF839496u.toInt(),
                STYLE_ANNOTATION to 0xFFB58900u.toInt(),
                STYLE_PREPROCESSOR to 0xFF859900u.toInt(),
                STYLE_MACRO to 0xFFCB4B16u.toInt(),
                STYLE_LIFETIME to 0xFFD33682u.toInt(),
                STYLE_SELECTOR to 0xFF268BD2u.toInt(),
                STYLE_BUILTIN to 0xFF268BD2u.toInt(),
                STYLE_URL to 0xFF268BD2u.toInt(),
                STYLE_PROPERTY to 0xFF268BD2u.toInt(),
            ))
        }

        private fun nord(): HighlightTheme {
            return HighlightTheme("Nord", 0xFF2E3440u.toInt(), 0xFFD8DEE9u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFF81A1C1u.toInt(),
                STYLE_STRING to 0xFFA3BE8Cu.toInt(),
                STYLE_NUMBER to 0xFFB48EADu.toInt(),
                STYLE_COMMENT to 0xFF616E88u.toInt(),
                STYLE_CLASS to 0xFF8FBCBBu.toInt(),
                STYLE_METHOD to 0xFF88C0D0u.toInt(),
                STYLE_VARIABLE to 0xFFD8DEE9u.toInt(),
                STYLE_PUNCTUATION to 0xFFECEFF4u.toInt(),
                STYLE_ANNOTATION to 0xFFEBCB8Bu.toInt(),
                STYLE_PREPROCESSOR to 0xFF81A1C1u.toInt(),
                STYLE_MACRO to 0xFFB48EADu.toInt(),
                STYLE_LIFETIME to 0xFFEBCB8Bu.toInt(),
                STYLE_SELECTOR to 0xFF8FBCBBu.toInt(),
                STYLE_BUILTIN to 0xFF5E81ACu.toInt(),
                STYLE_URL to 0xFF88C0D0u.toInt(),
                STYLE_PROPERTY to 0xFF88C0D0u.toInt(),
            ))
        }

        private fun githubDark(): HighlightTheme {
            return HighlightTheme("GitHub Dark", 0xFF0D1117u.toInt(), 0xFFC9D1D9u.toInt(), mapOf(
                STYLE_KEYWORD to 0xFFFF7B72u.toInt(),
                STYLE_STRING to 0xFFA5D6FFu.toInt(),
                STYLE_NUMBER to 0xFF79C0FFu.toInt(),
                STYLE_COMMENT to 0xFF8B949Eu.toInt(),
                STYLE_CLASS to 0xFFFFA657u.toInt(),
                STYLE_METHOD to 0xFFD2A8FFu.toInt(),
                STYLE_VARIABLE to 0xFFFFA657u.toInt(),
                STYLE_PUNCTUATION to 0xFFC9D1D9u.toInt(),
                STYLE_ANNOTATION to 0xFFFFA657u.toInt(),
                STYLE_PREPROCESSOR to 0xFFFF7B72u.toInt(),
                STYLE_MACRO to 0xFF79C0FFu.toInt(),
                STYLE_LIFETIME to 0xFFFFA657u.toInt(),
                STYLE_SELECTOR to 0xFF7EE787u.toInt(),
                STYLE_BUILTIN to 0xFF79C0FFu.toInt(),
                STYLE_URL to 0xFF79C0FFu.toInt(),
                STYLE_PROPERTY to 0xFF79C0FFu.toInt(),
            ))
        }
    }
}

fun Int.toColor(): Color {
    return Color(this)
}
