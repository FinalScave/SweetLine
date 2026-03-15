package com.qiplat.sweetline;

import java.util.ArrayList;
import java.util.List;

/**
 * 指定行区域高亮切片
 */
public class DocumentHighlightSlice {
    /**
     * 切片起始行
     */
    public int startLine;
    /**
     * patch 后文档总行数
     */
    public int totalLineCount;
    /**
     * 切片行高亮序列
     */
    public List<LineHighlight> lines = new ArrayList<>();
}
