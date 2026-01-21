package com.qiplat.sweetline;

import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.CharacterStyle;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

/**
 * 高亮分析器
 */
public class DocumentAnalyzer {
    protected long nativeHandle;

    protected DocumentAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * 对整个文本进行高亮分析
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyze() {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        return readDocumentHighlight(buffer, getHighlightConfig());
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param range patch的变更范围
     * @param newText patch的文本
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyzeChanges(TextRange range, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = ((long) range.start.line << 32) | (range.start.column & 0XFFFFFFFFL);
        long endRange = ((long) range.end.line << 32) | (range.end.column & 0XFFFFFFFFL);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return readDocumentHighlight(buffer, getHighlightConfig());
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @return 整个文本的高亮结果
     */
    public DocumentHighlight analyzeChanges(int startIndex, int endIndex, String newText) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return readDocumentHighlight(buffer, getHighlightConfig());
    }

    /**
     * 对指定行分析高亮
     * @param line 行号索引
     * @return 对应行的高亮内容
     */
    public LineHighlight analyzeLine(int line) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeLine(nativeHandle, line);
        return readLineHighlight(buffer, getHighlightConfig());
    }

    /**
     * 对整个文本进行分析，并将高亮结果转换为 {@link Spannable}
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeAsSpannable(StyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyze(nativeHandle);
        return readSpannable(buffer, styleFactory, getHighlightConfig());
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果，并将高亮结果转换为 {@link Spannable}
     * @param range patch的变更范围
     * @param newText patch的文本
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeChangesAsSpannable(TextRange range, String newText, StyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        long startRange = ((long) range.start.line << 32) | (range.start.column & 0XFFFFFFFFL);
        long endRange = ((long) range.end.line << 32) | (range.end.column & 0XFFFFFFFFL);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return readSpannable(buffer, styleFactory, getHighlightConfig());
    }

    /**
     * 根据patch内容重新分析整个文本的高亮结果，并将高亮结果转换为 {@link Spannable}
     * @param startIndex patch的起始索引
     * @param endIndex patch的结束索引
     * @param newText patch的文本
     * @param styleFactory Span样式函数，通过styleId创建对应Span样式
     * @return {@link Spannable}
     */
    public Spannable analyzeChangesAsSpannable(int startIndex, int endIndex, String newText, StyleFactory styleFactory) {
        if (nativeHandle == 0) {
            return null;
        }
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return readSpannable(buffer, styleFactory, getHighlightConfig());
    }

    /**
     * 获取托管文档对象
     * @return {@link Document}
     */
    public Document getDocument() {
        if (nativeHandle == 0) {
            return null;
        }
        return new Document(nativeGetDocument(nativeHandle));
    }

    /**
     * 获取当前高亮配置
     * @return {@link HighlightConfig}
     */
    public HighlightConfig getHighlightConfig() {
        if (nativeHandle == 0) {
            return null;
        }
        int bits = nativeGetHighlightConfig(nativeHandle);
        return HighlightConfig.fromNativeBits(bits);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (nativeHandle != 0) {
            nativeFinalizeAnalyzer(nativeHandle);
        }
        nativeHandle = 0;
    }

    private static int computeSpanBufferSize(HighlightConfig config) {
        int baseCount = 6;
        if (config.inlineStyle) {
            baseCount += 3;
        } else {
            baseCount += 1;
        }
        return baseCount;
    }

    private static DocumentHighlight readDocumentHighlight(int[] buffer, HighlightConfig config) {
        DocumentHighlight highlight = new DocumentHighlight();
        LineHighlight lineHighlight = new LineHighlight();
        int spanInts = computeSpanBufferSize(config);
        int spanCount = buffer.length / spanInts;
        int line = -1;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * spanInts;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (config.inlineStyle) {
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                inlineStyle.loadNativeTags(buffer[baseIndex + 8]);
            } else {
                styleId = buffer[baseIndex + 6];
            }

            if (startLine != line) {
                line = startLine;
                lineHighlight = new LineHighlight();
                highlight.lines.add(lineHighlight);
            }

            TextRange range = new TextRange(
                    new TextPosition(startLine, startColumn, startIndex),
                    new TextPosition(endLine, endColumn, endIndex)
            );
            if (inlineStyle != null) {
                lineHighlight.spans.add(new TokenSpan(range, inlineStyle));
            } else {
                lineHighlight.spans.add(new TokenSpan(range, styleId));
            }
        }
        return highlight;
    }

    private static LineHighlight readLineHighlight(int[] buffer, HighlightConfig config) {
        int spanInts = computeSpanBufferSize(config);
        int spanCount = buffer.length / spanInts;
        LineHighlight lineHighlight = new LineHighlight();
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * spanInts;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (config.inlineStyle) {
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                inlineStyle.loadNativeTags(buffer[baseIndex + 8]);
            } else {
                styleId = buffer[baseIndex + 6];
            }

            TextRange range = new TextRange(
                    new TextPosition(startLine, startColumn, startIndex),
                    new TextPosition(endLine, endColumn, endIndex)
            );

            if (inlineStyle != null) {
                lineHighlight.spans.add(new TokenSpan(range, inlineStyle));
            } else {
                lineHighlight.spans.add(new TokenSpan(range, styleId));
            }
        }
        return lineHighlight;
    }

    private Spannable readSpannable(int[] buffer, StyleFactory styleFactory, HighlightConfig config) {
        SpannableString result = new SpannableString(getDocument().getText());
        int spanInts = computeSpanBufferSize(config);
        int spanCount = buffer.length / spanInts;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * spanInts;
            int startIndex = buffer[baseIndex + 2];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (config.inlineStyle) {
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                inlineStyle.loadNativeTags(buffer[baseIndex + 8]);
            } else {
                styleId = buffer[baseIndex + 6];
            }

            CharacterStyle characterStyle;
            if (inlineStyle != null) {
                characterStyle =styleFactory.createCharacterStyle(inlineStyle);
            } else {
                characterStyle =styleFactory.createCharacterStyle(styleId);
            }
            result.setSpan(characterStyle, startIndex, endIndex, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        return result;
    }

    public interface StyleFactory {
        CharacterStyle createCharacterStyle(int styleId);
        CharacterStyle createCharacterStyle(InlineStyle inlineStyle);
    }

    @CriticalNative
    private static native void nativeFinalizeAnalyzer(long handle);
    @FastNative
    private static native int[] nativeAnalyze(long handle);
    @FastNative
    private static native int[] nativeAnalyzeChanges(long handle, long startPosition, long endPosition, String newText);
    @FastNative
    private static native int[] nativeAnalyzeChanges2(long handle, int startIndex, int endIndex, String newText);
    @FastNative
    private static native int[] nativeAnalyzeLine(long handle, int line);
    @CriticalNative
    private static native long nativeGetDocument(long handle);
    @CriticalNative
    private static native int nativeGetHighlightConfig(long handle);
}
