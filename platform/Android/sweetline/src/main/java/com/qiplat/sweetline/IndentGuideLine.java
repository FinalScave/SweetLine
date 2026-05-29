package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Single indent guide line (vertical line segment)
 */
public class IndentGuideLine {
    /**
     * Branch point (e.g. position of else/case)
     */
    public static class BranchPoint {
        public int line;
        public int column;

        public BranchPoint() {
        }

        public BranchPoint(int line, int column) {
            this.line = line;
            this.column = column;
        }
    }

    /**
     * Column of the guide line (character column)
     */
    public int column;
    /**
     * Start line number
     */
    public int startLine;
    /**
     * End line number
     */
    public int endLine;
    /**
     * Whether the guide continues from before the returned slice
     */
    public boolean continuesBefore;
    /**
     * Whether the guide continues after the returned slice
     */
    public boolean continuesAfter;
    /**
     * Branch point list (line/column positions of else/case etc.)
     */
    public List<BranchPoint> branches = new ArrayList<>();

    public IndentGuideLine() {
    }

    public IndentGuideLine(int column, int startLine, int endLine, boolean continuesBefore, boolean continuesAfter) {
        this.column = column;
        this.startLine = startLine;
        this.endLine = endLine;
        this.continuesBefore = continuesBefore;
        this.continuesAfter = continuesAfter;
    }
}
