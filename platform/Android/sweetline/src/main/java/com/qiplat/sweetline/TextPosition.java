package com.qiplat.sweetline;

/**
 * Text position descriptor
 */
public class TextPosition {
    /**
     * Line number (0-based)
     */
    public int line;
    /**
     * Column number (0-based)
     */
    public int column;
    /**
     * Character index in the full text (0-based)
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
