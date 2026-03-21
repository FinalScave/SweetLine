package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Highlight result for the entire document
 *
 * @param lines Highlight sequence for each line
 */
public record DocumentHighlight(List<LineHighlight> lines) {

    public DocumentHighlight() {
        this(new ArrayList<>());
    }
}
