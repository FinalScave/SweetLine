package com.qiplat.sweetline;

public class TextPosition {
    public int line;
    public int column;
    public int index;

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
