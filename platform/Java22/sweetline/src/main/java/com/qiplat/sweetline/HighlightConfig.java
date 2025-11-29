package com.qiplat.sweetline;

/**
 * Highlight configuration
 *
 * @param showIndex   Whether the analysis result includes character index
 * @param inlineStyle Whether to use inline styles instead of returning style IDs
 */
public record HighlightConfig(boolean showIndex, boolean inlineStyle) {

    public HighlightConfig() {
        this(false, false);
    }
}
