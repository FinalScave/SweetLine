package com.qiplat.sweetline;

/**
 * 文本行元数据信息
 */
public class TextLineInfo {
    /**
     * 行号索引
     */
    public int line;

    /**
     * 行起始高亮状态
     */
    public int startState;

    /**
     * 行在整个文本中起始字符偏移 (不是字节)，用于计算高亮块(TokenSpan) index，HighlightConfig中未开启 showIndex 时 无需该字段
     */
    public int startCharOffset;

    public TextLineInfo(int line, int startState) {
        this.line = line;
        this.startState = startState;
    }

    public TextLineInfo(int line, int startState, int startCharOffset) {
        this.line = line;
        this.startState = startState;
        this.startCharOffset = startCharOffset;
    }
}