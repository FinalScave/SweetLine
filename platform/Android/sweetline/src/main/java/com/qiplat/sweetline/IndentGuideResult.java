package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Indent guide analysis result
 */
public class IndentGuideResult {
    /**
     * All vertical guide lines
     */
    public List<IndentGuideLine> guideLines = new ArrayList<>();
    /**
     * Block state for each line
     */
    public List<LineScopeState> lineStates = new ArrayList<>();
}
