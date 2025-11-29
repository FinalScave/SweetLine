package com.qiplat.sweetline;

/**
 * Each highlight token span
 */
public class TokenSpan {
    /**
     * Highlight range
     */
    public TextRange range;
    /**
     * Highlight style ID (only in non-inlineStyle mode)
     */
    public int styleId;
    /**
     * Detailed style info for the token span (only in inlineStyle mode)
     */
    public InlineStyle inlineStyle;

    public TokenSpan(TextRange range, int styleId) {
        this.range = range;
        this.styleId = styleId;
    }

    public TokenSpan(TextRange range, InlineStyle inlineStyle) {
        this.range = range;
        this.inlineStyle = inlineStyle;
    }

    @Override
    public String toString() {
        if (inlineStyle != null) {
            return "TokenSpan{" +
                    "range=" + range +
                    ", inlineStyle=" + inlineStyle +
                    '}';
        } else {
            return "TokenSpan{" +
                    "range=" + range +
                    ", styleId=" + styleId +
                    '}';
        }
    }
}
