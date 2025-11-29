package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Indent guide analysis result
 *
 * @param guideLines All vertical guide lines
 * @param lineStates Block state for each line
 */
public record IndentGuideResult(List<IndentGuideLine> guideLines, List<LineScopeState> lineStates) {

    public IndentGuideResult() {
this(new ArrayList<>(), new ArrayList<>());
    }
}
