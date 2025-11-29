package com.qiplat.sweetline;

/**
 * Text line metadata for single-line analysis
 *
 * @param line            Line index
 * @param startState      Start highlight state of the line
 * @param startCharOffset Start character offset in the full text
 */
public record TextLineInfo(int line, int startState, int startCharOffset) {

    public TextLineInfo(int line, int startState) {
        this(line, startState, 0);
    }
}
