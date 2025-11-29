package com.qiplat.sweetline;

/**
 * Line scope state for indent guide analysis
 *
 * @param nestingLevel Nesting level of the line
 * @param scopeState   Scope state: 0=START, 1=END, 2=CONTENT
 * @param scopeColumn  Column of the scope marker
 * @param indentLevel  Indentation level of the line
 */
public record LineScopeState(int nestingLevel, int scopeState, int scopeColumn, int indentLevel) {
}
