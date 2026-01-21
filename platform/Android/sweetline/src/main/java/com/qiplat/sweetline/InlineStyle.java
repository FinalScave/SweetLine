package com.qiplat.sweetline;

/**
 * 语法规则中直接包含的样式定义
 */
public class InlineStyle {
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

    protected void loadNativeTags(int bits) {
        if ((bits & 1) != 0) {
            this.isBold = true;
        }
        if ((bits & (1 << 1)) != 0) {
            this.isItalic = true;
        }
    }

    @Override
    public String toString() {
        return "InlineStyle{" +
                "foreground=" + foreground +
                ", background=" + background +
                ", isBold=" + isBold +
                ", isItalic=" + isItalic +
                '}';
    }
}
