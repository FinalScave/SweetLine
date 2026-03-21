package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Highlight token span sequence for each line
 *
 * @param spans Highlight sequence
 */
public record LineHighlight(List<TokenSpan> spans) {

    public LineHighlight() {
        this(new ArrayList<>());
    }
}
