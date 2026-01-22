package com.qiplat.sweetline.util;

import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.CharacterStyle;

import com.qiplat.sweetline.DocumentHighlight;
import com.qiplat.sweetline.InlineStyle;
import com.qiplat.sweetline.LineAnalyzeResult;
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

    public static DocumentHighlight readDocumentHighlight(int[] buffer, int stride) {
        DocumentHighlight highlight = new DocumentHighlight();
        LineHighlight lineHighlight = new LineHighlight();
        int spanCount = buffer.length / stride;
        int line = -1;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride;
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

    public static Spannable readSpannable(String text, int[] buffer, int stride, SpannableStyleFactory styleFactory) {
        SpannableString result = new SpannableString(text);
        int spanCount = buffer.length / stride;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * stride;
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

    public static LineAnalyzeResult readLineAnalyzeResult(int[] buffer, int spanStride) {
        LineAnalyzeResult result = new LineAnalyzeResult();
        int spanCount = buffer[0];
        LineHighlight lineHighlight = new LineHighlight();
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * spanStride + 1;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];

            int styleId = 0;
            InlineStyle inlineStyle = null;
            if (spanStride > 7) {
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
        int baseIndex = spanCount * spanStride + 1;
        result.endState = buffer[baseIndex++];
        result.charCount = buffer[baseIndex];
        return result;
    }
}
