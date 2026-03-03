package com.qiplat.sweetline;

/**
 * 行作用域划线分析状态
 */
public class LineScopeState {
    /**
     * 行所处嵌套层级
     */
    public int nestingLevel;
    /**
     * 行所处作用域划线状态: 0=START, 1=END, 2=CONTENT
     */
    public int scopeState;
    /**
     * 作用域划线所处列
     */
    public int scopeColumn;
    /**
     * 该行缩进等级
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
