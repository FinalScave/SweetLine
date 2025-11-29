package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Highlight slice for the specified line range
 */
public class DocumentHighlightSlice {
    /**
     * Slice start line
     */
    public int startLine;
    /**
     * Total line count after patch
     */
    public int totalLineCount;
    /**
     * Highlight sequence for slice lines
     */
    public List<LineHighlight> lines = new ArrayList<>();
}
