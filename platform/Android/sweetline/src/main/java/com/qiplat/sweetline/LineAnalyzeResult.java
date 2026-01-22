package com.qiplat.sweetline;

/**
 * 单行语法高亮分析结果
 */
public class LineAnalyzeResult {
    /**
     * 当前行高亮序列
     */
    public LineHighlight highlight;

    /**
     * 行分析完毕后结束状态
     */
    public int endState;

    /**
     * 当前行总计分析的字符总数，不包含换行符
     */
    public int charCount;
}