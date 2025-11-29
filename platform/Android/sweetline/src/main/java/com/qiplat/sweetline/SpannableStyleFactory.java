package com.qiplat.sweetline;

import android.text.style.CharacterStyle;

public interface SpannableStyleFactory {
    CharacterStyle createCharacterStyle(int styleId);

    CharacterStyle createCharacterStyle(InlineStyle inlineStyle);
}