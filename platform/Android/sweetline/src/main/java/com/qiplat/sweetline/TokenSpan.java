package com.qiplat.sweetline;

/**
 * 每一个高亮块
 */
public class TokenSpan {
    /**
     * 高亮区域
     */
    public TextRange range;
    /**
     * 高亮样式ID (非 inlineStyle 模式下才有该字段)
     */
    public int styleId;
    /**
     * 高亮块样式详细信息，inlineStyle 模式下才有该字段
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
