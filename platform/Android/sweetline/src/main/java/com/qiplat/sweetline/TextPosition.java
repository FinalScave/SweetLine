package com.qiplat.sweetline;

/**
 * 文本位置描述
 */
public class TextPosition {
    /**
     * 文字所处行，起始为0
     */
    public int line;
    /**
     * 文字所处列，起始为0
     */
    public int column;
    /**
     * 文字在全文中的索引，起始为0
     */
    public int index;

    public TextPosition(int line, int column) {
        this.line = line;
        this.column = column;
    }

    public TextPosition(int line, int column, int index) {
        this.line = line;
        this.column = column;
        this.index = index;
    }

    @Override
    public String toString() {
        return "TextPosition{" +
                "line=" + line +
                ", column=" + column +
                ", index=" + index +
                '}';
    }
}
