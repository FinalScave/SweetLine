package com.qiplat.sweetline;

/**
 * Line range descriptor (0-based)
 */
public class LineRange {
    /**
     * Start line number
     */
    public int startLine;
    /**
     * Line count
     */
    public int lineCount;

    public LineRange() {
    }

    public LineRange(int startLine, int lineCount) {
        this.startLine = startLine;
        this.lineCount = lineCount;
    }
}
