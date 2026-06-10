package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Bracket pair analysis result.
 *
 * @param startLine      Actual start line of the returned slice
 * @param totalLineCount Total document line count
 * @param lines          Bracket tokens grouped by line
 */
public record BracketPairResult(int startLine, int totalLineCount, List<LineBracketPairs> lines) {

    public BracketPairResult() {
        this(0, 0, new ArrayList<>());
    }
}
