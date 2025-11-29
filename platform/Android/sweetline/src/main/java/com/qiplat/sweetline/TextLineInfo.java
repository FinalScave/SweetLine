package com.qiplat.sweetline;

/**
 * Text line metadata
 */
public class TextLineInfo {
    /**
     * Line index
     */
    public int line;

    /**
     * Start highlight state of the line
     */
    public int startState;

    /**
     * Start character offset in the full text (not bytes), used for computing TokenSpan index; not needed when showIndex is disabled in HighlightConfig
     */
    public int startCharOffset;

    public TextLineInfo(int line, int startState) {
        this.line = line;
        this.startState = startState;
    }

    public TextLineInfo(int line, int startState, int startCharOffset) {
        this.line = line;
        this.startState = startState;
        this.startCharOffset = startCharOffset;
    }
}