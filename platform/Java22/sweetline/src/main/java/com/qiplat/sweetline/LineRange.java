package com.qiplat.sweetline;

/**
 * Line range descriptor (0-based)
 *
 * @param startLine Start line number
 * @param lineCount Line count
 */
public record LineRange(int startLine, int lineCount) {
}
