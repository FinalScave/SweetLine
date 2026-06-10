package com.qiplat.sweetline;

/**
 * Bracket match state.
 */
public enum BracketMatchState {
    MATCHED(0),
    UNMATCHED(1),
    UNKNOWN(2);

    private final int value;

    BracketMatchState(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static BracketMatchState fromValue(int value) {
        for (BracketMatchState state : values()) {
            if (state.value == value) {
                return state;
            }
        }
        return UNKNOWN;
    }
}
