package com.qiplat.sweetline;

public class HighlightConfig {
    public boolean showIndex;

    public static HighlightConfig withIndex() {
        HighlightConfig config = new HighlightConfig();
        config.showIndex = true;
        return config;
    }
}
