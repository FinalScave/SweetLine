package com.qiplat.sweetline;

/**
 * Bracket token kind.
 */
public enum BracketTokenKind {
    OPEN(0),
    CLOSE(1);

    private final int value;

    BracketTokenKind(int value) {
        this.value = value;
    }

    public int value() {
        return value;
    }

    public static BracketTokenKind fromValue(int value) {
        for (BracketTokenKind kind : values()) {
            if (kind.value == value) {
                return kind;
            }
        }
        return OPEN;
    }
}
