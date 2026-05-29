package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Indent guide analysis result
 *
 * @param startLine  Actual start line of the returned slice
 * @param guideLines All vertical guide lines
 * @param lineStates Block state for each line
 */
public record IndentGuideResult(int startLine, List<IndentGuideLine> guideLines, List<LineScopeState> lineStates) {

    public IndentGuideResult() {
        this(0, new ArrayList<>(), new ArrayList<>());
    }
}
