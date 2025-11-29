package com.qiplat.sweetline;

/**
 * Inline style definition embedded in syntax rules
 *
 * @param foreground      Foreground color
 * @param background      Background color
 * @param isBold          Whether to display in bold
 * @param isItalic        Whether to display in italic
 * @param isStrikethrough Whether to display with strikethrough
 */
public record InlineStyle(int foreground, int background, boolean isBold, boolean isItalic, boolean isStrikethrough) {

    /** Font attribute bitmask for bold */
    public static final int STYLE_BOLD = 1;
    /** Font attribute bitmask for italic */
    public static final int STYLE_ITALIC = STYLE_BOLD << 1;
    /** Font attribute bitmask for strikethrough */
    public static final int STYLE_STRIKE_THROUGH = STYLE_ITALIC << 1;

    /**
     * Construct from raw font attribute bitmask
     */
    public InlineStyle(int foreground, int background, int fontAttributes) {
        this(foreground, background,
                (fontAttributes & STYLE_BOLD) != 0,
                (fontAttributes & STYLE_ITALIC) != 0,
                (fontAttributes & STYLE_STRIKE_THROUGH) != 0);
    }
}
