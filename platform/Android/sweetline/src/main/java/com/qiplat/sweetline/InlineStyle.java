package com.qiplat.sweetline;

/**
 * Inline style definition embedded in syntax rules
 */
public class InlineStyle {
    public static final int STYLE_BOLD = 1;
    public static final int STYLE_ITALIC = STYLE_BOLD << 1;
    public static final int STYLE_STRIKE_THROUGH = STYLE_ITALIC << 1;

    /**
     * Foreground color
     */
    public int foreground;
    /**
     * Foreground color
     */
    public int background;
    /**
     * Whether to display in bold
     */
    public boolean isBold;
    /**
     * Whether to display in italic
     */
    public boolean isItalic;
    /**
     * Whether to display with strikethrough
     */
    public boolean isStrikethrough;

    @Override
    public String toString() {
        return "InlineStyle{" +
                "foreground=" + foreground +
                ", background=" + background +
                ", isBold=" + isBold +
                ", isItalic=" + isItalic +
                ", isStrikethrough=" + isStrikethrough +
                '}';
    }
}
