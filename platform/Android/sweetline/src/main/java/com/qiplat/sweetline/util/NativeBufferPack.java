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
        if (buffer == null || buffer.length < 3) {
            return highlight;
        }
        int flags = buffer[0];
        int stride = Math.max(buffer[1], 0);
        int lineCount = Math.max(buffer[2], 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean isInlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, isInlineStyle)) {
            return highlight;
        }
        int index = 3;
        int spanFieldCount = spanFieldCount(stride);
        for (int line = 0; line < lineCount && index < buffer.length; line++) {
            LineHighlight lineHighlight = new LineHighlight();
            highlight.lines.add(lineHighlight);
            int spanCount = Math.max(buffer[index++], 0);
            for (int i = 0; i < spanCount; i++) {
                if (index + spanFieldCount > buffer.length) {
                    return highlight;
                }
                int startColumn = buffer[index++];
                int length = buffer[index++];
                int startIndex = hasStartIndex ? buffer[index++] : 0;
                int endColumn = startColumn + length;
                int endIndex = hasStartIndex ? startIndex + length : 0;

                TextRange range = new TextRange(
                        new TextPosition(line, startColumn, startIndex),
                        new TextPosition(line, endColumn, endIndex)
                );

                if (isInlineStyle) {
                    InlineStyle inlineStyle = new InlineStyle();
                    inlineStyle.foreground = buffer[index++];
                    inlineStyle.background = buffer[index++];
                    unpackInlineStyleTags(buffer[index++], inlineStyle);
                    lineHighlight.spans.add(new TokenSpan(range, inlineStyle));
                } else {
                    int styleId = buffer[index++];
                    lineHighlight.spans.add(new TokenSpan(range, styleId));
                }
            }
        }
        return highlight;
    }

    public static DocumentHighlightSlice readDocumentHighlightSlice(int[] buffer) {
        DocumentHighlightSlice slice = new DocumentHighlightSlice();
        if (buffer == null || buffer.length < 5) {
            return slice;
        }
        int flags = buffer[0];
        int stride = Math.max(buffer[1], 0);
        slice.startLine = buffer[2];
        slice.totalLineCount = buffer[3];
        int lineCount = Math.max(buffer[4], 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean isInlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, isInlineStyle)) {
            return slice;
        }
        int spanFieldCount = spanFieldCount(stride);
        int index = 5;
        for (int i = 0; i < lineCount; i++) {
            slice.lines.add(new LineHighlight());
            if (index >= buffer.length) {
                break;
            }
            int spanCount = Math.max(buffer[index++], 0);
            int line = slice.startLine + i;
            LineHighlight lineHighlight = slice.lines.get(i);
            for (int s = 0; s < spanCount; s++) {
                if (index + spanFieldCount > buffer.length) {
                    return slice;
                }
                int startColumn = buffer[index++];
                int length = buffer[index++];
                int startIndex = hasStartIndex ? buffer[index++] : 0;
                int endColumn = startColumn + length;
                int endIndex = hasStartIndex ? startIndex + length : 0;

                TextRange range = new TextRange(
                        new TextPosition(line, startColumn, startIndex),
                        new TextPosition(line, endColumn, endIndex)
                );
                if (isInlineStyle) {
                    InlineStyle inlineStyle = new InlineStyle();
                    inlineStyle.foreground = buffer[index++];
                    inlineStyle.background = buffer[index++];
                    unpackInlineStyleTags(buffer[index++], inlineStyle);
                    lineHighlight.spans.add(new TokenSpan(range, inlineStyle));
                } else {
                    int styleId = buffer[index++];
                    lineHighlight.spans.add(new TokenSpan(range, styleId));
                }
            }
        }
        return slice;
    }

    public static Spannable readSpannable(String text, int[] buffer, SpannableStyleFactory styleFactory) {
        SpannableString result = new SpannableString(text);
        if (buffer == null || buffer.length < 3) {
            return result;
        }
        int flags = buffer[0];
        int stride = Math.max(buffer[1], 0);
        int lineCount = Math.max(buffer[2], 0);
        boolean hasStartIndex = flagsHasStartIndex(flags);
        if (!hasStartIndex) {
            throw new IllegalStateException("Cannot readSpannable() without startIndex of TokenSpan");
        }
        boolean isInlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, isInlineStyle)) {
            return result;
        }
        int spanFieldCount = spanFieldCount(stride);
        int index = 3;
        for (int line = 0; line < lineCount && index < buffer.length; line++) {
            int spanCount = Math.max(buffer[index++], 0);
            for (int i = 0; i < spanCount; i++) {
                if (index + spanFieldCount > buffer.length) {
                    return result;
                }
                int column = buffer[index++];
                int length = buffer[index++];
                int startIndex = buffer[index++];
                int endIndex = startIndex + length;

                CharacterStyle characterStyle;
                if (isInlineStyle) {
                    InlineStyle inlineStyle = new InlineStyle();
                    inlineStyle.foreground = buffer[index++];
                    inlineStyle.background = buffer[index++];
                    unpackInlineStyleTags(buffer[index++], inlineStyle);
                    characterStyle = styleFactory.createCharacterStyle(inlineStyle);
                } else {
                    int styleId = buffer[index++];
                    characterStyle = styleFactory.createCharacterStyle(styleId);
                }

                if (startIndex < 0 || endIndex < startIndex || endIndex > text.length()) {
                    continue;
                }
                result.setSpan(characterStyle, startIndex, endIndex, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        }
        return result;
    }

    public static LineAnalyzeResult readLineAnalyzeResult(int[] buffer, int lineNumber) {
        LineAnalyzeResult result = new LineAnalyzeResult();
        if (buffer == null || buffer.length < 5) {
            return result;
        }
        int flags = buffer[0];
        int stride = buffer[1];
        int spanCount = buffer[2];
        result.endState = buffer[3];
        result.charCount = buffer[4];
        boolean hasStartIndex = flagsHasStartIndex(flags);
        boolean isInlineStyle = flagsUsesInlineStyle(flags);
        if (!isValidSpanStride(stride, hasStartIndex, isInlineStyle)) {
            return result;
        }
        int spanFieldCount = spanFieldCount(stride);
        LineHighlight lineHighlight = new LineHighlight();
        int index = 5;
        for (int i = 0; i < spanCount; i++) {
            if (index + spanFieldCount > buffer.length) {
                break;
            }
            int startColumn = buffer[index++];
            int length = buffer[index++];
            int startIndex = hasStartIndex ? buffer[index++] : 0;
            int endColumn = startColumn + length;
            int endIndex = hasStartIndex ? startIndex + length : 0;

            TextRange range = new TextRange(
                    new TextPosition(lineNumber, startColumn, startIndex),
                    new TextPosition(lineNumber, endColumn, endIndex)
            );

            if (isInlineStyle) {
                InlineStyle inlineStyle = new InlineStyle();
                inlineStyle.foreground = buffer[index++];
                inlineStyle.background = buffer[index++];
                unpackInlineStyleTags(buffer[index++], inlineStyle);
                lineHighlight.spans.add(new TokenSpan(range, inlineStyle));
            } else {
                int styleId = buffer[index++];
                lineHighlight.spans.add(new TokenSpan(range, styleId));
            }
        }
        result.highlight = lineHighlight;
        return result;
    }

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

    private static boolean isValidSpanStride(int stride, boolean hasStartIndex, boolean isInlineStyle) {
        int expected = 2 + (hasStartIndex ? 1 : 0) + (isInlineStyle ? 3 : 1);
        return stride == expected;
    }

    private static boolean flagsUsesInlineStyle(int flags) {
        return (flags & (1 << 1)) != 0;
    }

    private static boolean flagsHasStartIndex(int flags) {
        return (flags & 1) != 0;
    }

    private static int spanFieldCount(int stride) {
        return stride;
    }
}
