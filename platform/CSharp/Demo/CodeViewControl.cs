using System.Drawing.Drawing2D;
using SweetLine;

namespace Demo;

internal sealed class CodeViewControl : Control
{
    private const int LineNumberPadding = 12;
    private const int CodeLeftPadding = 8;
    private static readonly TextFormatFlags TextFlags = TextFormatFlags.NoPadding | TextFormatFlags.NoPrefix;

    private readonly Font _codeFont = new("Consolas", 11.5f, FontStyle.Regular, GraphicsUnit.Point);

    private string? _sourceCode;
    private DocumentHighlight? _highlight;
    private IndentGuideResult? _indentGuides;
    private HighlightTheme? _theme;

    public CodeViewControl()
    {
        SetStyle(
            ControlStyles.AllPaintingInWmPaint |
            ControlStyles.OptimizedDoubleBuffer |
            ControlStyles.UserPaint |
            ControlStyles.ResizeRedraw,
            true);

        Font = _codeFont;
        Size = new Size(400, 300);
    }

    public void SetTheme(HighlightTheme theme)
    {
        _theme = theme;
        Invalidate();
    }

    public void SetHighlightData(string sourceCode, DocumentHighlight highlight, IndentGuideResult indentGuides)
    {
        _sourceCode = sourceCode;
        _highlight = highlight;
        _indentGuides = indentGuides;
        UpdateCanvasSize();
        Invalidate();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);

        HighlightTheme? currentTheme = _theme;
        if (currentTheme is null)
        {
            return;
        }

        Graphics g = e.Graphics;
        g.SmoothingMode = SmoothingMode.AntiAlias;
        g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;

        Color bgColor = ArgbToColor(currentTheme.BackgroundColor);
        Color textColor = ArgbToColor(currentTheme.TextColor);
        using SolidBrush defaultBrush = new(textColor);

        g.Clear(bgColor);

        if (_sourceCode is null || _highlight is null)
        {
            DrawText(g, "Select a file to highlight", _codeFont, textColor, 20, 20);
            return;
        }

        string[] lines = SplitLines(_sourceCode);
        int lineHeight = MeasureTextHeight(g, _codeFont);
        int lineNumWidth = MeasureTextWidth(g, lines.Length.ToString(), _codeFont) + LineNumberPadding * 2;
        int charWidth = MeasureTextWidth(g, " ", _codeFont);
        if (charWidth <= 0)
        {
            charWidth = 8;
        }

        Color gutterBg = BlendColor(bgColor, 0.05f);
        using (SolidBrush gutterBrush = new(gutterBg))
        {
            g.FillRectangle(gutterBrush, 0, 0, lineNumWidth, Height);
        }

        using (Pen sepPen = new(BlendColor(bgColor, 0.12f)))
        {
            g.DrawLine(sepPen, lineNumWidth, 0, lineNumWidth, Height);
        }

        int codeX = lineNumWidth + CodeLeftPadding;
        Dictionary<int, LineHighlight> lineMap = BuildLineMap(_highlight);

        if (_indentGuides is not null)
        {
            DrawIndentGuides(g, lineHeight, codeX, charWidth, lines.Length, currentTheme, _indentGuides);
        }

        Color lineNumColor = BlendColor(textColor, bgColor, 0.5f);
        for (int lineNum = 0; lineNum < lines.Length; lineNum++)
        {
            int y = lineNum * lineHeight;

            string lineNumText = (lineNum + 1).ToString();
            int numWidth = MeasureTextWidth(g, lineNumText, _codeFont);
            int lineNumX = Math.Max(0, lineNumWidth - LineNumberPadding - numWidth);
            DrawText(g, lineNumText, _codeFont, lineNumColor, lineNumX, y);

            string lineText = lines[lineNum];
            if (!lineMap.TryGetValue(lineNum, out LineHighlight? lineHighlight))
            {
                DrawText(g, lineText, _codeFont, textColor, codeX, y);
                continue;
            }

            DrawHighlightedLine(g, lineText, lineHighlight, currentTheme, textColor, codeX, y, lineHeight);
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _codeFont.Dispose();
        }

        base.Dispose(disposing);
    }

    private void UpdateCanvasSize()
    {
        if (_sourceCode is null)
        {
            Size = new Size(400, 300);
            return;
        }

        using Graphics g = CreateGraphics();
        string[] lines = SplitLines(_sourceCode);
        int lineHeight = MeasureTextHeight(g, _codeFont);

        int maxWidth = 0;
        foreach (string line in lines)
        {
            int width = MeasureTextWidth(g, line, _codeFont);
            maxWidth = Math.Max(maxWidth, width);
        }

        int lineNumWidth = MeasureTextWidth(g, lines.Length.ToString(), _codeFont) + LineNumberPadding * 2;
        int contentWidth = lineNumWidth + CodeLeftPadding + maxWidth + 24;
        int contentHeight = lines.Length * lineHeight + 12;
        Size = new Size(Math.Max(contentWidth, 400), Math.Max(contentHeight, 300));
    }

    private static string[] SplitLines(string text)
    {
        return text.Replace("\r\n", "\n").Split('\n');
    }

    private static Dictionary<int, LineHighlight> BuildLineMap(DocumentHighlight highlight)
    {
        Dictionary<int, LineHighlight> map = [];
        foreach (LineHighlight line in highlight.Lines)
        {
            if (line.Spans.Count == 0)
            {
                continue;
            }

            int actualLine = line.Spans[0].Range.Start.Line;
            map[actualLine] = line;
        }

        return map;
    }

    private void DrawHighlightedLine(
        Graphics g,
        string lineText,
        LineHighlight lineHighlight,
        HighlightTheme theme,
        Color defaultColor,
        int x,
        int lineTop,
        int lineHeight)
    {
        int lastCol = 0;
        foreach (TokenSpan span in lineHighlight.Spans)
        {
            int startCol = Math.Max(span.Range.Start.Column, 0);
            int endCol = span.Range.Start.Line == span.Range.End.Line
                ? Math.Min(span.Range.End.Column, lineText.Length)
                : lineText.Length;

            if (startCol > lastCol && lastCol < lineText.Length)
            {
                string gap = lineText.Substring(lastCol, Math.Min(startCol, lineText.Length) - lastCol);
                DrawText(g, gap, _codeFont, defaultColor, x, lineTop);
                x += MeasureTextWidth(g, gap, _codeFont);
            }

            if (startCol < lineText.Length && endCol > startCol)
            {
                string tokenText = lineText.Substring(startCol, Math.Min(endCol, lineText.Length) - startCol);

                int argbColor;
                bool bold = false;
                bool italic = false;
                bool strike = false;
                if (span.InlineStyle is not null)
                {
                    argbColor = span.InlineStyle.Foreground != 0 ? span.InlineStyle.Foreground : theme.TextColor;
                    bold = span.InlineStyle.IsBold;
                    italic = span.InlineStyle.IsItalic;
                    strike = span.InlineStyle.IsStrikethrough;
                }
                else
                {
                    argbColor = theme.GetColor(span.StyleId);
                }

                FontStyle style = FontStyle.Regular;
                if (bold)
                {
                    style |= FontStyle.Bold;
                }

                if (italic)
                {
                    style |= FontStyle.Italic;
                }

                Font drawFont = _codeFont;
                bool disposeTempFont = false;
                if (style != FontStyle.Regular)
                {
                    try
                    {
                        drawFont = new Font(_codeFont, style);
                        disposeTempFont = true;
                    }
                    catch
                    {
                        drawFont = _codeFont;
                    }
                }

                int tokenWidth;
                try
                {
                    DrawText(g, tokenText, drawFont, ArgbToColor(argbColor), x, lineTop);
                    tokenWidth = MeasureTextWidth(g, tokenText, drawFont);
                }
                finally
                {
                    if (disposeTempFont)
                    {
                        drawFont.Dispose();
                    }
                }

                if (strike)
                {
                    int strikeY = lineTop + lineHeight / 2;
                    using Pen strikePen = new(ArgbToColor(argbColor));
                    g.DrawLine(strikePen, x, strikeY, x + tokenWidth, strikeY);
                }

                x += tokenWidth;
            }

            lastCol = endCol;
        }

        if (lastCol < lineText.Length)
        {
            DrawText(g, lineText[lastCol..], _codeFont, defaultColor, x, lineTop);
        }
    }

    private static void DrawIndentGuides(
        Graphics g,
        int lineHeight,
        int codeX,
        int charWidth,
        int totalLines,
        HighlightTheme theme,
        IndentGuideResult guides)
    {
        Color guideColor = BlendColor(ArgbToColor(theme.TextColor), ArgbToColor(theme.BackgroundColor), 0.35f);
        using Pen pen = new(guideColor, 1f) { DashPattern = [2f, 3f] };

        foreach (IndentGuideLine guide in guides.GuideLines)
        {
            int innerStart = guide.StartLine + 1;
            int innerEnd = guide.EndLine - 1;
            if (innerStart > innerEnd)
            {
                continue;
            }

            int x = codeX + guide.Column * charWidth;
            int y1 = innerStart * lineHeight;
            int y2 = Math.Min(innerEnd, totalLines - 1) * lineHeight + lineHeight;
            g.DrawLine(pen, x, y1, x, y2);
        }
    }

    private static Color ArgbToColor(int argb)
    {
        return Color.FromArgb(argb);
    }

    private static Color BlendColor(Color color, float brighten)
    {
        int r = Math.Min(255, (int)(color.R + 255 * brighten));
        int g = Math.Min(255, (int)(color.G + 255 * brighten));
        int b = Math.Min(255, (int)(color.B + 255 * brighten));
        return Color.FromArgb(r, g, b);
    }

    private static Color BlendColor(Color c1, Color c2, float ratio)
    {
        int r = (int)(c1.R * ratio + c2.R * (1f - ratio));
        int g = (int)(c1.G * ratio + c2.G * (1f - ratio));
        int b = (int)(c1.B * ratio + c2.B * (1f - ratio));
        return Color.FromArgb(Clamp(r), Clamp(g), Clamp(b));
    }

    private static int Clamp(int value)
    {
        return Math.Max(0, Math.Min(255, value));
    }

    private static int MeasureTextHeight(Graphics g, Font font)
    {
        int h = TextRenderer.MeasureText(g, "A", font, new Size(int.MaxValue, int.MaxValue), TextFlags).Height;
        return h > 0 ? h : font.Height + 2;
    }

    private static int MeasureTextWidth(Graphics g, string text, Font font)
    {
        if (string.IsNullOrEmpty(text))
        {
            return 0;
        }

        return TextRenderer.MeasureText(g, text, font, new Size(int.MaxValue, int.MaxValue), TextFlags).Width;
    }

    private static void DrawText(Graphics g, string text, Font font, Color color, int x, int y)
    {
        if (string.IsNullOrEmpty(text))
        {
            return;
        }

        TextRenderer.DrawText(g, text, font, new Point(x, y), color, TextFlags);
    }

    private static void DrawText(Graphics g, string text, Font font, Color color, float x, float y)
    {
        DrawText(g, text, font, color, (int)MathF.Round(x), (int)MathF.Round(y));
    }
}
