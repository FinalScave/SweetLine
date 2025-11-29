package com.qiplat.sweetline;

/**
 * Text range descriptor
 */
public class TextRange {
    /**
     * Start position
     */
    public TextPosition start;
    /**
     * End position
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
