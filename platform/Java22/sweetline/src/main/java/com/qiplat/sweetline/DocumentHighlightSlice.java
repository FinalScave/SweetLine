package com.qiplat.sweetline;

import java.util.List;

/**
 * Highlight slice for the specified line range
 *
 * @param startLine      Slice start line
 * @param totalLineCount Total line count after patch
 * @param lines          Highlight sequence for slice lines
 */
public record DocumentHighlightSlice(int startLine, int totalLineCount, List<LineHighlight> lines) {
}
