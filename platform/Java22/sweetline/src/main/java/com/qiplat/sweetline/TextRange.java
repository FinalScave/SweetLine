package com.qiplat.sweetline;

/**
 * Text range descriptor
 *
 * @param start Start position
 * @param end   End position
 */
public record TextRange(TextPosition start, TextPosition end) {
}
