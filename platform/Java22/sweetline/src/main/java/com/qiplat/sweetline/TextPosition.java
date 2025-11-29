package com.qiplat.sweetline;

/**
 * Text position descriptor
 *
 * @param line   Line number (0-based)
 * @param column Column number (0-based)
 * @param index  Character index in the full text (0-based)
 */
public record TextPosition(int line, int column, int index) {

    public TextPosition(int line, int column) {
        this(line, column, 0);
    }
}
