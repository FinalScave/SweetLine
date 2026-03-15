package com.qiplat.sweetline.util;

import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.CharacterStyle;

import com.qiplat.sweetline.DocumentHighlight;
import com.qiplat.sweetline.DocumentHighlightSlice;
import com.qiplat.sweetline.IndentGuideLine;
import com.qiplat.sweetline.IndentGuideResult;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.LineAnalyzeResult;
import com.qiplat.sweetline.LineScopeState;
import com.qiplat.sweetline.LineHighlight;
import com.qiplat.sweetline.SpannableStyleFactory;
import com.qiplat.sweetline.TextLineInfo;
import com.qiplat.sweetline.TextPosition;
import com.qiplat.sweetline.TextRange;
import com.qiplat.sweetline.TokenSpan;

public final class NativeBufferPack {
    public static long packTextPosition(TextPosition position) {
        return ((long) position.line << 32) | (position.column & 0XFFFFFFFFL);
    }

    public static TextPosition unpackTextPosition(long bits) {
        int line = (int) (bits >> 32);
        int column = (int) (bits & 0XFFFFFFFFL);
        return new TextPosition(line, column);
    }

    public static int[] packTextLineInfo(TextLineInfo info) {
        int[] arr = new int[3];
        arr[0] = info.line;
        arr[1] = info.startState;
        arr[2] = info.startCharOffset;
        return arr;
    }

    public static void unpackInlineStyleTags(int tags, InlineStyle inlineStyle) {
        if ((tags & InlineStyle.STYLE_BOLD) != 0) {
            inlineStyle.isBold = true;
        }
        if ((tags & InlineStyle.STYLE_ITALIC) != 0) {
            inlineStyle.isItalic = true;
        }
        if ((tags & InlineStyle.STYLE_STRIKE_THROUGH) != 0) {
            inlineStyle.isStrikethrough = true;
        }
    }

    public static DocumentHighlight readDocumentHighlight(int[] buffer) {
        DocumentHighlight highlight = new DocumentHighlight();
        if (buffer == null || buffer.length < 2) {
            return highlight;
        }
        LineHighlight lineHighlight = new LineHighlight();
        int spanCount = buffer[0];
        int stride = buffer[1];
        int line = -1;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 2;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (stride > 7) {
                if (baseIndex + 8 >= buffer.length) {
                    break;
                }
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
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

    /**
     * 从 int[] 缓冲区中解析指定行区域高亮切片
     * 布局:
     * buffer[0] = startLine
     * buffer[1] = totalLineCount
     * buffer[2] = lineCount
     * buffer[3] = spanCount
     * buffer[4] = stride
     * 之后是 spanCount * stride 的高亮块数据
     */
    public static DocumentHighlightSlice readDocumentHighlightSlice(int[] buffer) {
        DocumentHighlightSlice slice = new DocumentHighlightSlice();
        if (buffer == null || buffer.length < 5) {
            return slice;
        }
        slice.startLine = buffer[0];
        slice.totalLineCount = buffer[1];
        int lineCount = Math.max(buffer[2], 0);
        int spanCount = Math.max(buffer[3], 0);
        int stride = Math.max(buffer[4], 0);
        for (int i = 0; i < lineCount; i++) {
            slice.lines.add(new LineHighlight());
        }
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 5;
            if (baseIndex + 6 >= buffer.length) {
                break;
            }
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (stride > 7) {
                if (baseIndex + 8 >= buffer.length) {
                    break;
                }
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
            } else {
                styleId = buffer[baseIndex + 6];
            }

            int localLine = startLine - slice.startLine;
            if (localLine < 0 || localLine >= slice.lines.size()) {
                continue;
            }
            TextRange range = new TextRange(
                    new TextPosition(startLine, startColumn, startIndex),
                    new TextPosition(endLine, endColumn, endIndex)
            );
            if (inlineStyle != null) {
                slice.lines.get(localLine).spans.add(new TokenSpan(range, inlineStyle));
            } else {
                slice.lines.get(localLine).spans.add(new TokenSpan(range, styleId));
            }
        }
        return slice;
    }

    public static Spannable readSpannable(String text, int[] buffer, SpannableStyleFactory styleFactory) {
        SpannableString result = new SpannableString(text);
        int spanCount = buffer[0];
        int stride = buffer[1];
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 2;
            int startIndex = buffer[baseIndex + 2];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (stride > 7) {
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
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

    public static LineAnalyzeResult readLineAnalyzeResult(int[] buffer) {
        LineAnalyzeResult result = new LineAnalyzeResult();
        int spanCount = buffer[0];
        int stride = buffer[1];
        result.endState = buffer[2];
        result.charCount = buffer[3];
        LineHighlight lineHighlight = new LineHighlight();
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride + 4;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (stride > 7) {
                inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[baseIndex + 6];
                inlineStyle.background = buffer[baseIndex + 7];
                unpackInlineStyleTags(buffer[baseIndex + 8], inlineStyle);
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
        result.highlight = lineHighlight;
        return result;
    }

    /**
     * 从 int[] 缓冲区中解析缩进划线分析结果
     * 布局:
     * buffer[0] = guide_lines 数量
     * buffer[1] = 每条 guide_line 的固定字段数 (stride=6)
     * buffer[2] = line_states 数量
     * buffer[3] = 每行 line_state 的字段数 (4)
     * 之后依次: guide_lines 数据, line_states 数据
     */
    public static IndentGuideResult readIndentGuideResult(int[] buffer) {
        IndentGuideResult result = new IndentGuideResult();
        if (buffer == null || buffer.length < 4) {
            return result;
        }
        int guideCount = buffer[0];
        // buffer[1] = stride (unused, we read dynamically)
        int lineStateCount = buffer[2];
        int lineStateStride = buffer[3];

        int idx = 4;
        for (int i = 0; i < guideCount; i++) {
            IndentGuideLine guide = new IndentGuideLine();
            guide.column = buffer[idx++];
            guide.startLine = buffer[idx++];
            guide.endLine = buffer[idx++];
            guide.nestingLevel = buffer[idx++];
            guide.scopeRuleId = buffer[idx++];
            int branchCount = buffer[idx++];
            for (int j = 0; j < branchCount; j++) {
                IndentGuideLine.BranchPoint bp = new IndentGuideLine.BranchPoint();
                bp.line = buffer[idx++];
                bp.column = buffer[idx++];
                guide.branches.add(bp);
            }
            result.guideLines.add(guide);
        }
        for (int i = 0; i < lineStateCount; i++) {
            LineScopeState state = new LineScopeState();
            state.nestingLevel = buffer[idx++];
            state.scopeState = buffer[idx++];
            state.scopeColumn = buffer[idx++];
            state.indentLevel = buffer[idx++];
            result.lineStates.add(state);
        }
        return result;
    }
}
