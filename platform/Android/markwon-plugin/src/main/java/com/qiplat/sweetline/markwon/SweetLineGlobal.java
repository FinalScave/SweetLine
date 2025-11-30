package com.qiplat.sweetline.markwon;

import com.qiplat.sweetline.HighlightConfig;
import com.qiplat.sweetline.HighlightEngine;

public class SweetLineGlobal {
    private static final HighlightEngine ENGINE = new HighlightEngine(HighlightConfig.withIndex());

    public static HighlightEngine getEngineInstance() {
        return ENGINE;
    }
}
