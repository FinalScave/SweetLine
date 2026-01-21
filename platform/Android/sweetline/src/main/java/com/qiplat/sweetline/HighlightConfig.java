package com.qiplat.sweetline;

/**
 * 高亮配置
 */
public class HighlightConfig {
    /**
     * 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
     */
    public boolean showIndex;
    /**
     * 是否支持内联样式，即不需要外部注册高亮样式，直接在语法规则json中定义高亮样式，高亮分析结果中直接包含高亮样式(前景色、加粗等），而不是返回样式ID
     */
    public boolean inlineStyle;

    public HighlightConfig() {
    }

    public HighlightConfig(boolean showIndex, boolean inlineStyle) {
        this.showIndex = showIndex;
        this.inlineStyle = inlineStyle;
    }

    protected int toNativeBits() {
        int bits = 0;
        if (showIndex) {
            bits |= 1;
        }
        if (inlineStyle) {
            bits |= 1 << 1;
        }
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
        return config;
    }
}
