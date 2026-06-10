package com.qiplat.sweetline;

/**
 * Single bracket token.
 *
 * @param range        Bracket token range
 * @param depth        Nesting depth used by rainbow bracket rendering
 * @param kind         Token kind
 * @param matchState   Match state
 * @param partnerRange Matched partner range, or null when unavailable
 */
public record BracketToken(
        TextRange range,
        int depth,
        BracketTokenKind kind,
        BracketMatchState matchState,
        TextRange partnerRange) {
}
