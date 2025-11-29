package com.qiplat.sweetline;

/**
 * Single line syntax highlight analysis result
 *
 * @param highlight Highlight sequence of the current line
 * @param endState  End state after line analysis
 * @param charCount Total character count analyzed in the current line
 */
public record LineAnalyzeResult(LineHighlight highlight, int endState, int charCount) {
}
