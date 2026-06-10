package com.qiplat.sweetline;

public class BracketToken {
    public static final int KIND_OPEN = 0;
    public static final int KIND_CLOSE = 1;

    public static final int MATCHED = 0;
    public static final int UNMATCHED = 1;
    public static final int UNKNOWN = 2;

    public TextRange range;
    public int depth;
    public int kind;
    public int matchState;
    public TextRange partnerRange;

    public BracketToken() {
    }

    public BracketToken(TextRange range, int depth, int kind, int matchState, TextRange partnerRange) {
        this.range = range;
        this.depth = depth;
        this.kind = kind;
        this.matchState = matchState;
        this.partnerRange = partnerRange;
    }
}
