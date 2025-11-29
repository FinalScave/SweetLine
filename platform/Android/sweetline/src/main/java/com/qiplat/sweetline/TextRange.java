package com.qiplat.sweetline;

public class TextRange {
    public TextPosition start;
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
