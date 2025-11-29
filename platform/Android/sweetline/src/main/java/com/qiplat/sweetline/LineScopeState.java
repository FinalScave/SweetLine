package com.qiplat.sweetline;

/**
 * Line scope state for indent guide analysis
 */
public class LineScopeState {
    /**
     * Nesting level of the line
     */
    public int nestingLevel;
    /**
     * Scope state: 0=START, 1=END, 2=CONTENT
     */
    public int scopeState;
    /**
     * Column of the scope marker
     */
    public int scopeColumn;
    /**
     * Indentation level of the line
     */
    public int indentLevel;

    public LineScopeState() {
    }

    public LineScopeState(int nestingLevel, int scopeState, int scopeColumn, int indentLevel) {
        this.nestingLevel = nestingLevel;
        this.scopeState = scopeState;
        this.scopeColumn = scopeColumn;
        this.indentLevel = indentLevel;
    }
}
