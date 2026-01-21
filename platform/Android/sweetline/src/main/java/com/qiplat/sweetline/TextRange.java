package com.qiplat.sweetline;

/**
 * 文字的范围区间描述
 */
public class TextRange {
    /**
     * 起始位置
     */
    public TextPosition start;
    /**
     * 结束位置
     */
    public TextPosition end;

    public TextRange(TextPosition start, TextPosition end) {
        this.start = start;
        this.end = end;
    }

    @Override
    public String toString() {
        return "TextRange{" +
                "start=" + start +
                ", end=" + end +
                '}';
    }
}
