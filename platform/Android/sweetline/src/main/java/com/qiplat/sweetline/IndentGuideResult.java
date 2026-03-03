package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * 缩进划线分析结果
 */
public class IndentGuideResult {
    /**
     * 所有纵向划线
     */
    public List<IndentGuideLine> guideLines = new ArrayList<>();
    /**
     * 每行的块状态
     */
    public List<LineScopeState> lineStates = new ArrayList<>();
}
