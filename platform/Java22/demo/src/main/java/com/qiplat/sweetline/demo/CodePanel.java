package com.qiplat.sweetline.demo;

import com.qiplat.sweetline.*;

import javax.swing.*;
import java.awt.*;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Custom panel that renders syntax-highlighted source code with line numbers and indent guides.
 */
public class CodePanel extends JPanel {

    private static final int LINE_NUMBER_PADDING = 12;
    private static final int CODE_LEFT_PADDING = 8;
    private static final float FONT_SIZE = 14f;

    private final Font codeFont;

    private String sourceCode;
    private DocumentHighlight highlight;
    private IndentGuideResult indentGuides;
    private BracketPairResult bracketPairs;
    private HighlightTheme theme;
    private static final Color[] BRACKET_PALETTE = {
            new Color(0xFF7DD3FC, true),
            new Color(0xFFF9A8D4, true),
            new Color(0xFFFDE047, true),
            new Color(0xFF86EFAC, true),
            new Color(0xFFC4B5FD, true),
            new Color(0xFFFDBA74, true)
    };

    public CodePanel() {
        codeFont = new Font(Font.SANS_SERIF, Font.PLAIN, 1).deriveFont(FONT_SIZE);
        setFont(codeFont);
    }

    public void setTheme(HighlightTheme theme) {
        this.theme = theme;
    }

    public void setHighlightData(String sourceCode, DocumentHighlight highlight, IndentGuideResult indentGuides,
                                 BracketPairResult bracketPairs) {
        this.sourceCode = sourceCode;
        this.highlight = highlight;
        this.indentGuides = indentGuides;
        this.bracketPairs = bracketPairs;
    }

    @Override
    public Dimension getPreferredSize() {
        if (sourceCode == null) {
            return new Dimension(400, 300);
        }
        FontMetrics fm = getFontMetrics(codeFont);
        String[] lines = sourceCode.split("\n", -1);
        int lineHeight = fm.getHeight();
        int maxWidth = 0;
        for (String line : lines) {
            maxWidth = Math.max(maxWidth, fm.stringWidth(line));
        }
        int lineNumWidth = fm.stringWidth(String.valueOf(lines.length)) + LINE_NUMBER_PADDING * 2;
        return new Dimension(lineNumWidth + CODE_LEFT_PADDING + maxWidth + 20,
                lines.length * lineHeight + 10);
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g;
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

        HighlightTheme currentTheme = this.theme;
        if (currentTheme == null) {
            return;
        }

        Color bgColor = argbToColor(currentTheme.backgroundColor);
        Color textColor = argbToColor(currentTheme.textColor);

        g2.setColor(bgColor);
        g2.fillRect(0, 0, getWidth(), getHeight());

        if (sourceCode == null || highlight == null) {
            g2.setColor(textColor);
            g2.setFont(codeFont);
            g2.drawString("Select a file to highlight", 20, 30);
            return;
        }

        g2.setFont(codeFont);
        FontMetrics fm = g2.getFontMetrics();
        int lineHeight = fm.getHeight();
        int ascent = fm.getAscent();

        String[] lines = sourceCode.split("\n", -1);
        int lineNumWidth = fm.stringWidth(String.valueOf(lines.length)) + LINE_NUMBER_PADDING * 2;

        // Line number gutter background
        Color gutterBg = blendColor(bgColor, 0.05f);
        g2.setColor(gutterBg);
        g2.fillRect(0, 0, lineNumWidth, getHeight());

        // Separator line between gutter and code area
        g2.setColor(blendColor(bgColor, 0.12f));
        g2.drawLine(lineNumWidth, 0, lineNumWidth, getHeight());

        int codeX = lineNumWidth + CODE_LEFT_PADDING;

        // Build line-to-highlight map
        Map<Integer, LineHighlight> lineHighlightMap = new LinkedHashMap<>();
        for (LineHighlight lh : highlight.lines()) {
            if (!lh.spans().isEmpty()) {
                int actualLine = lh.spans().getFirst().range().start().line();
                lineHighlightMap.put(actualLine, lh);
            }
        }

        // Draw indent guides first (behind text)
        if (indentGuides != null) {
            drawIndentGuides(g2, fm, codeX, lineHeight, lines.length, currentTheme);
        }

        // Draw line numbers and code text
        Color lineNumColor = blendColor(textColor, bgColor, 0.5f);
        for (int lineNum = 0; lineNum < lines.length; lineNum++) {
            int y = lineNum * lineHeight;
            int baselineY = y + ascent;

            String numStr = String.valueOf(lineNum + 1);
            int numWidth = fm.stringWidth(numStr);
            g2.setColor(lineNumColor);
            g2.drawString(numStr, lineNumWidth - LINE_NUMBER_PADDING - numWidth, baselineY);

            String lineText = lines[lineNum];
            LineHighlight lineHighlight = lineHighlightMap.get(lineNum);
            LineBracketPairs linePairs = getBracketLine(lineNum);
            if (lineHighlight == null) {
                drawTextWithBracketColors(g2, lineText, 0, lineText.length(),
                        textColor, codeFont, codeX, baselineY, currentTheme, linePairs);
            } else {
                drawHighlightedLine(g2, fm, lineText, lineHighlight, currentTheme,
                        textColor, codeX, baselineY, linePairs);
            }
        }
    }

    private void drawHighlightedLine(Graphics2D g2, FontMetrics fm, String lineText,
                                     LineHighlight lineHighlight, HighlightTheme theme,
                                     Color defaultColor, int x, int baselineY,
                                     LineBracketPairs linePairs) {
        int lastCol = 0;
        for (TokenSpan span : lineHighlight.spans()) {
            int startCol = span.range().start().column();
            int endCol = (span.range().start().line() == span.range().end().line())
                    ? Math.min(span.range().end().column(), lineText.length())
                    : lineText.length();

            if (startCol > lastCol && lastCol < lineText.length()) {
                x = drawTextWithBracketColors(g2, lineText, lastCol, Math.min(startCol, lineText.length()),
                        defaultColor, codeFont, x, baselineY, theme, linePairs);
            }

            int argbColor;
            boolean bold = false;
            boolean italic = false;
            boolean strikethrough = false;
            if (span.inlineStyle() != null) {
                argbColor = span.inlineStyle().foreground() != 0
                        ? span.inlineStyle().foreground()
                        : theme.textColor;
                bold = span.inlineStyle().isBold();
                italic = span.inlineStyle().isItalic();
                strikethrough = span.inlineStyle().isStrikethrough();
            } else {
                argbColor = theme.getColor(span.styleId());
            }

            if (startCol < lineText.length() && endCol > startCol) {
                Font origFont = g2.getFont();
                Font drawFont = origFont;
                if (bold || italic) {
                    int style = (bold ? Font.BOLD : 0) | (italic ? Font.ITALIC : 0);
                    drawFont = origFont.deriveFont(style);
                }
                int beforeX = x;
                x = drawTextWithBracketColors(g2, lineText, startCol, Math.min(endCol, lineText.length()),
                        argbToColor(argbColor), drawFont, x, baselineY, theme, linePairs);

                if (strikethrough) {
                    int strikeY = baselineY - fm.getAscent() / 3;
                    g2.setColor(argbToColor(argbColor));
                    g2.drawLine(beforeX, strikeY, x, strikeY);
                }
            }
            lastCol = endCol;
        }
        if (lastCol < lineText.length()) {
            drawTextWithBracketColors(g2, lineText, lastCol, lineText.length(),
                    defaultColor, codeFont, x, baselineY, theme, linePairs);
        }
    }

    private int drawTextWithBracketColors(Graphics2D g2, String lineText, int startCol, int endCol,
                                          Color baseColor, Font font, int x, int baselineY,
                                          HighlightTheme theme, LineBracketPairs linePairs) {
        int start = clamp(startCol, 0, lineText.length());
        int end = clamp(endCol, start, lineText.length());
        if (end <= start) {
            return x;
        }
        if (linePairs == null || linePairs.tokens().isEmpty()) {
            return drawTextRun(g2, lineText.substring(start, end), font, baseColor, x, baselineY);
        }

        int cursor = start;
        for (BracketToken token : linePairs.tokens()) {
            int tokenStart = clamp(token.range().start().column(), 0, lineText.length());
            int tokenEnd = clamp(token.range().end().column(), tokenStart, lineText.length());
            if (tokenEnd <= cursor || tokenStart >= end) {
                continue;
            }

            int clippedStart = Math.max(cursor, Math.max(start, tokenStart));
            int clippedEnd = Math.min(end, tokenEnd);
            if (clippedStart > cursor) {
                x = drawTextRun(g2, lineText.substring(cursor, clippedStart), font, baseColor, x, baselineY);
            }
            if (clippedEnd > clippedStart) {
                x = drawTextRun(g2, lineText.substring(clippedStart, clippedEnd),
                        font, bracketColor(token, theme), x, baselineY);
            }
            cursor = Math.max(cursor, clippedEnd);
        }

        if (cursor < end) {
            x = drawTextRun(g2, lineText.substring(cursor, end), font, baseColor, x, baselineY);
        }
        return x;
    }

    private int drawTextRun(Graphics2D g2, String text, Font font, Color color, int x, int baselineY) {
        if (text.isEmpty()) {
            return x;
        }
        Font originalFont = g2.getFont();
        g2.setFont(font);
        g2.setColor(color);
        g2.drawString(text, x, baselineY);
        int width = g2.getFontMetrics(font).stringWidth(text);
        g2.setFont(originalFont);
        return x + width;
    }

    private LineBracketPairs getBracketLine(int lineNum) {
        if (bracketPairs == null) {
            return null;
        }
        int lineIndex = lineNum - bracketPairs.startLine();
        return lineIndex >= 0 && lineIndex < bracketPairs.lines().size()
                ? bracketPairs.lines().get(lineIndex)
                : null;
    }

    private static Color bracketColor(BracketToken token, HighlightTheme theme) {
        if (token.matchState() == BracketMatchState.UNMATCHED) {
            return new Color(0xFFFF6B6B, true);
        }
        Color color = BRACKET_PALETTE[Math.floorMod(token.depth(), BRACKET_PALETTE.length)];
        return token.matchState() == BracketMatchState.UNKNOWN
                ? blendColor(color, argbToColor(theme.backgroundColor), 0.68f)
                : color;
    }

    private void drawIndentGuides(Graphics2D g2, FontMetrics fm, int codeX,
                                  int lineHeight, int totalLines, HighlightTheme theme) {
        Color guideColor = blendColor(argbToColor(theme.textColor),
                argbToColor(theme.backgroundColor), 0.35f);
        g2.setColor(guideColor);

        float[] dash = {2f, 3f};
        Stroke guideStroke = new BasicStroke(1f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 1f, dash, 0f);
        Stroke origStroke = g2.getStroke();
        g2.setStroke(guideStroke);

        int charWidth = fm.charWidth(' ');
        for (IndentGuideLine guide : indentGuides.guideLines()) {
            int innerStart = guide.startLine() + 1;
            int innerEnd = guide.endLine() - 1;
            if (innerStart > innerEnd) {
                continue;
            }
            int x = codeX + guide.column() * charWidth;
            int y1 = innerStart * lineHeight;
            int y2 = Math.min(innerEnd, totalLines - 1) * lineHeight + lineHeight;
            g2.drawLine(x, y1, x, y2);
        }

        g2.setStroke(origStroke);
    }

    private static Color argbToColor(int argb) {
        int r = (argb >> 16) & 0xFF;
        int g = (argb >> 8) & 0xFF;
        int b = argb & 0xFF;
        return new Color(r, g, b);
    }

    private static Color blendColor(Color c1, float brighten) {
        int r = Math.min(255, (int) (c1.getRed() + 255 * brighten));
        int g = Math.min(255, (int) (c1.getGreen() + 255 * brighten));
        int b = Math.min(255, (int) (c1.getBlue() + 255 * brighten));
        return new Color(r, g, b);
    }

    private static Color blendColor(Color c1, Color c2, float ratio) {
        int r = (int) (c1.getRed() * ratio + c2.getRed() * (1 - ratio));
        int g = (int) (c1.getGreen() * ratio + c2.getGreen() * (1 - ratio));
        int b = (int) (c1.getBlue() * ratio + c2.getBlue() * (1 - ratio));
        return new Color(clamp(r), clamp(g), clamp(b));
    }

    private static int clamp(int v) {
        return Math.max(0, Math.min(255, v));
    }

    private static int clamp(int v, int min, int max) {
        return Math.max(min, Math.min(max, v));
    }
}
