package com.qiplat.sweetline;

/**
 * Each highlight token span
 *
 * @param range       Highlight range
 * @param styleId     Highlight style ID (only in non-inlineStyle mode, -1 otherwise)
 * @param inlineStyle Detailed style info (only in inlineStyle mode, null otherwise)
 */
public record TokenSpan(TextRange range, int styleId, InlineStyle inlineStyle) {

    /**
     * Construct with style ID (non-inline mode)
     */
    public TokenSpan(TextRange range, int styleId) {
        this(range, styleId, null);
    }

    /**
     * Construct with inline style
     */
    public TokenSpan(TextRange range, InlineStyle inlineStyle) {
        this(range, -1, inlineStyle);
    }
}
