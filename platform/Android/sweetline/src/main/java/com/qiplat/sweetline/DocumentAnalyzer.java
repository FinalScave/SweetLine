package com.qiplat.sweetline;

import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.CharacterStyle;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

public class DocumentAnalyzer {
    protected long nativeHandle;

    protected DocumentAnalyzer(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }
    public DocumentHighlight analyze() {
        int[] buffer = nativeAnalyze(nativeHandle);
        return readDocumentHighlight(buffer);
    }

    public DocumentHighlight analyzeChanges(TextRange range, String newText) {
        long startRange = ((long) range.start.line << 32) | (range.start.column & 0XFFFFFFFFL);
        long endRange = ((long) range.end.line << 32) | (range.end.column & 0XFFFFFFFFL);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return readDocumentHighlight(buffer);
    }

    public DocumentHighlight analyzeChanges(int startIndex, int endIndex, String newText) {
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return readDocumentHighlight(buffer);
    }

    public LineHighlight analyzeLine(int line) {
        int[] buffer = nativeAnalyzeLine(nativeHandle, line);
        return readLineHighlight(buffer);
    }

    public Spannable analyzeAsSpannable(StyleFactory styleFactory) {
        int[] buffer = nativeAnalyze(nativeHandle);
        return readSpannable(buffer, styleFactory);
    }

    public Spannable analyzeChangesAsSpannable(TextRange range, String newText, StyleFactory styleFactory) {
        long startRange = ((long) range.start.line << 32) | (range.start.column & 0XFFFFFFFFL);
        long endRange = ((long) range.end.line << 32) | (range.end.column & 0XFFFFFFFFL);
        int[] buffer = nativeAnalyzeChanges(nativeHandle, startRange, endRange, newText);
        return readSpannable(buffer, styleFactory);
    }

    public Spannable analyzeChangesAsSpannable(int startIndex, int endIndex, String newText, StyleFactory styleFactory) {
        int[] buffer = nativeAnalyzeChanges2(nativeHandle, startIndex, endIndex, newText);
        return readSpannable(buffer, styleFactory);
    }

    public Document getDocument() {
        return new Document(nativeGetDocument(nativeHandle));
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        nativeFinalizeAnalyzer(nativeHandle);
        nativeHandle = 0;
    }

    private static DocumentHighlight readDocumentHighlight(int[] buffer) {
        DocumentHighlight highlight = new DocumentHighlight();
        LineHighlight lineHighlight = new LineHighlight();
        int spanCount = buffer.length / 7;
        int line = -1;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * 7;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];
            int styleId = buffer[baseIndex + 6];

            if (startLine != line) {
                line = startLine;
                lineHighlight = new LineHighlight();
                highlight.lines.add(lineHighlight);
            }

            TextRange range = new TextRange(
                    new TextPosition(startLine, startColumn, startIndex),
                    new TextPosition(endLine, endColumn, endIndex)
            );
            TokenSpan span = new TokenSpan(range, styleId);
            lineHighlight.spans.add(span);
        }
        return highlight;
    }

    private static LineHighlight readLineHighlight(int[] buffer) {
        int spanCount = buffer.length / 7;
        LineHighlight lineHighlight = new LineHighlight();
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * 7;
            int startLine = buffer[baseIndex];
            int startColumn = buffer[baseIndex + 1];
            int startIndex = buffer[baseIndex + 2];
            int endLine = buffer[baseIndex + 3];
            int endColumn = buffer[baseIndex + 4];
            int endIndex = buffer[baseIndex + 5];
            int styleId = buffer[baseIndex + 6];
            TextRange range = new TextRange(
                    new TextPosition(startLine, startColumn, startIndex),
                    new TextPosition(endLine, endColumn, endIndex)
            );
            TokenSpan span = new TokenSpan(range, styleId);
            lineHighlight.spans.add(span);
        }
        return lineHighlight;
    }

    private Spannable readSpannable(int[] buffer, StyleFactory styleFactory) {
        SpannableString result = new SpannableString(getDocument().getText());
        int spanCount = buffer.length / 7;
        for (int i = 0; i < spanCount; i++) {
            int baseIndex = i * 7;
            int startIndex = buffer[baseIndex + 2];
            int endIndex = buffer[baseIndex + 5];
            int styleId = buffer[baseIndex + 6];
            CharacterStyle characterStyle = styleFactory.createCharacterStyle(styleId);
            result.setSpan(characterStyle, startIndex, endIndex, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        return result;
    }

    public interface StyleFactory {
        CharacterStyle createCharacterStyle(int styleId);
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
}
