package com.qiplat.sweetline;

/**
 * 行范围描述（0-based）
 */
public class LineRange {
    /**
     * 起始行号
     */
    public int startLine;
    /**
     * 行数量
     */
    public int lineCount;

    public LineRange() {
    }

    public LineRange(int startLine, int lineCount) {
        this.startLine = startLine;
        this.lineCount = lineCount;
    }
}
