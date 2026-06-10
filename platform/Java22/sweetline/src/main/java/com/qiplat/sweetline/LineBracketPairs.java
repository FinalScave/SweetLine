package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * Bracket token sequence for each line.
 *
 * @param tokens Bracket token sequence
 */
public record LineBracketPairs(List<BracketToken> tokens) {

    public LineBracketPairs() {
        this(new ArrayList<>());
    }
}
