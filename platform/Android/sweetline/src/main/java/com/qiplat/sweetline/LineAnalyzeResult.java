package com.qiplat.sweetline;

/**
 * Single line syntax highlight analysis result
 */
public class LineAnalyzeResult {
    /**
     * Highlight sequence of the current line
     */
    public LineHighlight highlight;

    /**
     * End state after line analysis
     */
    public int endState;

    /**
     * Total character count analyzed in the current line, excluding line ending
     */
    public int charCount;
}