package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Single indent guide line (vertical line segment)
 *
 * @param column       Column of the guide line (character column)
 * @param startLine    Start line number
 * @param endLine      End line number
 * @param nestingLevel Nesting level (0-based)
 * @param scopeRuleId  Associated ScopeRule ID (matching pair mode), -1 for indentation mode
 * @param branches     Branch point list (positions of else/case etc.)
 */
public record IndentGuideLine(int column, int startLine, int endLine, int nestingLevel, int scopeRuleId,
                              List<BranchPoint> branches) {

    /**
     * Branch point (e.g. position of else/case)
     */
    public record BranchPoint(int line, int column) {
    }

    public IndentGuideLine(int column, int startLine, int endLine, int nestingLevel, int scopeRuleId) {
        this(column, startLine, endLine, nestingLevel, scopeRuleId, new ArrayList<>());
    }
}
