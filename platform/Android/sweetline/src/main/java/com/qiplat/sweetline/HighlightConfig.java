package com.qiplat.sweetline;

/**
 * Highlight configuration
 */
public class HighlightConfig {
    /**
     * Whether the analysis result includes character index; without it, each TokenSpan only has line and column
     */
    public boolean showIndex;
    /**
     * Whether to use inline styles, i.e. style definitions are embedded directly in syntax rule JSON, and the analysis result contains style info instead of returning style IDs
     */
    public boolean inlineStyle;
    /**
     * Tab width for indent guide level calculation (1 tab = tabSize spaces), default 4
     */
    public int tabSize = 4;

    public HighlightConfig() {
    }

    public HighlightConfig(boolean showIndex, boolean inlineStyle) {
        this.showIndex = showIndex;
        this.inlineStyle = inlineStyle;
    }

    public HighlightConfig(boolean showIndex, boolean inlineStyle, int tabSize) {
        this.showIndex = showIndex;
        this.inlineStyle = inlineStyle;
        this.tabSize = tabSize;
    }

    protected int toNativeBits() {
        int bits = 0;
        if (showIndex) {
            bits |= 1;
        }
        if (inlineStyle) {
            bits |= 1 << 1;
        }
        // Encode tabSize into bit8~bit15 (8 bits, supports 0~255)
        bits |= (tabSize & 0xFF) << 8;
        return bits;
    }

    protected static HighlightConfig fromNativeBits(int bits) {
        HighlightConfig config = new HighlightConfig();
        if ((bits & 1) != 0) {
            config.showIndex = true;
        }
        if ((bits & (1 << 1)) != 0) {
            config.inlineStyle = true;
        }
        int tabSize = (bits >> 8) & 0xFF;
        if (tabSize > 0) {
            config.tabSize = tabSize;
        }
        return config;
    }
}
