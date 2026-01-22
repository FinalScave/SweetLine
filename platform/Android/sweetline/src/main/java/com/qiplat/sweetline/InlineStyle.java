package com.qiplat.sweetline;

/**
 * 语法规则中直接包含的样式定义
 */
public class InlineStyle {
    public static final int STYLE_BOLD = 1;
    public static final int STYLE_ITALIC = STYLE_BOLD << 1;
    public static final int STYLE_STRIKE_THROUGH = STYLE_ITALIC << 1;

    /**
     * 前景色
     */
    public int foreground;
    /**
     * 前景色
     */
    public int background;
    /**
     * 是否加粗显示
     */
    public boolean isBold;
    /**
     * 是否斜体显示
     */
    public boolean isItalic;
    /**
     * 是否需要显示删除线
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
