package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * 单条缩进划线（纵向线段）
 */
public class IndentGuideLine {
    /**
     * 分支点（如 else/case 的位置）
     */
    public static class BranchPoint {
        public int line;
        public int column;

        public BranchPoint() {
        }

        public BranchPoint(int line, int column) {
            this.line = line;
            this.column = column;
        }
    }

    /**
     * 划线所在列（字符列）
     */
    public int column;
    /**
     * 起始行号
     */
    public int startLine;
    /**
     * 结束行号
     */
    public int endLine;
    /**
     * 嵌套层级（0-based）
     */
    public int nestingLevel;
    /**
     * 关联的 ScopeRule id（匹配对模式），-1=缩进模式
     */
    public int scopeRuleId;
    /**
     * 分支点列表（如 else/case 的行列位置）
     */
    public List<BranchPoint> branches = new ArrayList<>();

    public IndentGuideLine() {
    }

    public IndentGuideLine(int column, int startLine, int endLine, int nestingLevel, int scopeRuleId) {
        this.column = column;
        this.startLine = startLine;
        this.endLine = endLine;
        this.nestingLevel = nestingLevel;
        this.scopeRuleId = scopeRuleId;
    }
}
