package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Single indent guide line (vertical line segment)
 *
 * @param column       Column of the guide line (character column)
 * @param startLine    Start line number
 * @param endLine      End line number
 * @param continuesBefore Whether the guide continues from before the returned slice
 * @param continuesAfter  Whether the guide continues after the returned slice
 * @param branches     Branch point list (positions of else/case etc.)
 */
public record IndentGuideLine(int column, int startLine, int endLine, boolean continuesBefore, boolean continuesAfter,
                              List<BranchPoint> branches) {

    /**
     * Branch point (e.g. position of else/case)
     */
    public record BranchPoint(int line, int column) {
    }

    public IndentGuideLine(int column, int startLine, int endLine, boolean continuesBefore, boolean continuesAfter) {
        this(column, startLine, endLine, continuesBefore, continuesAfter, new ArrayList<>());
    }
}
