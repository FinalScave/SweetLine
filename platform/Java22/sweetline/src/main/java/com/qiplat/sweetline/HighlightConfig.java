package com.qiplat.sweetline;

/**
 * Highlight configuration
 *
 * @param showIndex   Whether the analysis result includes character index
 * @param inlineStyle Whether to use inline styles instead of returning style IDs
 * @param tabSize     Tab width used for indent guide level calculation
 */
public record HighlightConfig(boolean showIndex, boolean inlineStyle, int tabSize) {

    public HighlightConfig() {
        this(false, false, 4);
    }
}
