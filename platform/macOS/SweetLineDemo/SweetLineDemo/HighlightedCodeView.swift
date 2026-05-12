import AppKit
import SwiftUI
import SweetLine

struct HighlightedCodeView: NSViewRepresentable {
    let source: String
    let highlight: DocumentHighlight?
    let indentGuides: IndentGuideResult?
    let theme: HighlightTheme

    static func preferredSize(source: String, minimumSize: CGSize) -> CGSize {
        HighlightedCodeNSView.preferredSize(source: source, minimumSize: minimumSize)
    }

    func makeNSView(context: Context) -> HighlightedCodeNSView {
        HighlightedCodeNSView()
    }

    func updateNSView(_ nsView: HighlightedCodeNSView, context: Context) {
        nsView.source = source
        nsView.highlight = highlight
        nsView.indentGuides = indentGuides
        nsView.theme = theme
    }
}

final class HighlightedCodeNSView: NSView {
    private static let lineNumberPadding: CGFloat = 12
    private static let codeLeftPadding: CGFloat = 8
    private static let textTopPadding: CGFloat = 6
    private static let textBottomPadding: CGFloat = 10
    private static let rightPadding: CGFloat = 24
    private static let codeFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)

    var source = "" { didSet { needsDisplay = true } }
    var highlight: DocumentHighlight? { didSet { needsDisplay = true } }
    var indentGuides: IndentGuideResult? { didSet { needsDisplay = true } }
    var theme = HighlightTheme.sweetLineDark() { didSet { needsDisplay = true } }

    override var isFlipped: Bool { true }

    static func preferredSize(source: String, minimumSize: CGSize) -> CGSize {
        let lines = splitLines(source)
        let attributes: [NSAttributedString.Key: Any] = [.font: codeFont]
        let lineHeight = codeFont.ascender - codeFont.descender + codeFont.leading
        let maxLineWidth = lines.reduce(CGFloat(0)) { width, line in
            max(width, NSString(string: line).size(withAttributes: attributes).width)
        }
        let lineNumberWidth = NSString(string: String(max(lines.count, 1))).size(withAttributes: attributes).width + lineNumberPadding * 2
        let width = lineNumberWidth + codeLeftPadding + maxLineWidth + rightPadding
        let height = textTopPadding + CGFloat(max(lines.count, 1)) * lineHeight + textBottomPadding
        return CGSize(width: max(width, minimumSize.width), height: max(height, minimumSize.height))
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        let lines = Self.splitLines(source)
        let lineHeight = Self.codeFont.ascender - Self.codeFont.descender + Self.codeFont.leading
        let attributes: [NSAttributedString.Key: Any] = [.font: Self.codeFont]
        let lineNumberWidth = NSString(string: String(max(lines.count, 1))).size(withAttributes: attributes).width + Self.lineNumberPadding * 2
        let codeX = lineNumberWidth + Self.codeLeftPadding

        theme.backgroundNSColor.setFill()
        dirtyRect.fill()

        drawGutter(lineNumberWidth: lineNumberWidth)
        drawIndentGuides(codeX: codeX, lineHeight: lineHeight, totalLines: lines.count)

        if source.isEmpty {
            drawString("Select a file to highlight", at: CGPoint(x: 20, y: 24), color: theme.textNSColor, font: Self.codeFont)
            return
        }

        let lineNumberColor = blend(theme.textNSColor, with: theme.backgroundNSColor, ratio: 0.5)
        for lineIndex in lines.indices {
            let y = Self.textTopPadding + CGFloat(lineIndex) * lineHeight
            let baseline = y + Self.codeFont.ascender
            let lineNumber = String(lineIndex + 1)
            let lineNumberWidthActual = NSString(string: lineNumber).size(withAttributes: attributes).width
            drawString(lineNumber, at: CGPoint(x: lineNumberWidth - Self.lineNumberPadding - lineNumberWidthActual, y: baseline), color: lineNumberColor, font: Self.codeFont)

            let spans = highlightLine(lineIndex)?.spans ?? []
            drawCodeLine(lines[lineIndex], spans: spans, line: lineIndex, x: codeX, baseline: baseline)
        }
    }

    private func drawGutter(lineNumberWidth: CGFloat) {
        blend(theme.backgroundNSColor, with: .white, ratio: 0.95).setFill()
        NSRect(x: 0, y: 0, width: lineNumberWidth, height: bounds.height).fill()
        blend(theme.backgroundNSColor, with: .white, ratio: 0.88).setStroke()
        let path = NSBezierPath()
        path.move(to: CGPoint(x: lineNumberWidth, y: 0))
        path.line(to: CGPoint(x: lineNumberWidth, y: bounds.height))
        path.stroke()
    }

    private func drawIndentGuides(codeX: CGFloat, lineHeight: CGFloat, totalLines: Int) {
        guard let indentGuides else { return }

        let guideColor = blend(theme.textNSColor, with: theme.backgroundNSColor, ratio: 0.35)
        guideColor.setStroke()
        let charWidth = NSString(string: " ").size(withAttributes: [.font: Self.codeFont]).width

        let path = NSBezierPath()
        path.lineWidth = 1
        path.setLineDash([2, 3], count: 2, phase: 0)

        for guide in indentGuides.guideLines {
            let innerStart = guide.startLine + 1
            let innerEnd = guide.endLine - 1
            guard innerStart <= innerEnd else { continue }

            let x = codeX + CGFloat(guide.column) * charWidth
            let y1 = Self.textTopPadding + CGFloat(innerStart) * lineHeight
            let y2 = Self.textTopPadding + CGFloat(min(innerEnd, max(totalLines - 1, 0))) * lineHeight + lineHeight
            path.move(to: CGPoint(x: x, y: y1))
            path.line(to: CGPoint(x: x, y: y2))
        }

        path.stroke()
    }

    private func drawCodeLine(_ lineText: String, spans: [TokenSpan], line: Int, x: CGFloat, baseline: CGFloat) {
        var currentX = x
        var lastColumn = 0
        let maxColumn = lineText.count

        for span in spans {
            let startColumn = max(0, min(span.range.start.column, maxColumn))
            let endColumn = max(startColumn, min(span.range.end.column, maxColumn))

            if startColumn > lastColumn {
                currentX += drawSegment(lineText, start: lastColumn, end: startColumn, x: currentX, baseline: baseline, color: theme.textNSColor, font: Self.codeFont)
            }

            if endColumn > startColumn {
                let tokenFont = font(for: span.style)
                let tokenColor = theme.foreground(for: span.style)
                let width = drawSegment(lineText, start: startColumn, end: endColumn, x: currentX, baseline: baseline, color: tokenColor, font: tokenFont)
                if case let .inline(style) = span.style, style.isStrikethrough {
                    tokenColor.setStroke()
                    let y = baseline - Self.codeFont.ascender / 3
                    let path = NSBezierPath()
                    path.move(to: CGPoint(x: currentX, y: y))
                    path.line(to: CGPoint(x: currentX + width, y: y))
                    path.stroke()
                }
                currentX += width
            }

            lastColumn = endColumn
        }

        if lastColumn < maxColumn {
            _ = drawSegment(lineText, start: lastColumn, end: maxColumn, x: currentX, baseline: baseline, color: theme.textNSColor, font: Self.codeFont)
        }
    }

    private func drawSegment(_ text: String, start: Int, end: Int, x: CGFloat, baseline: CGFloat, color: NSColor, font: NSFont) -> CGFloat {
        let segment = substring(text, start: start, end: end)
        let size = NSString(string: segment).size(withAttributes: [.font: font])
        drawString(segment, at: CGPoint(x: x, y: baseline), color: color, font: font)
        return size.width
    }

    private func drawString(_ text: String, at point: CGPoint, color: NSColor, font: NSFont) {
        NSString(string: text).draw(
            at: CGPoint(x: point.x, y: point.y - font.ascender),
            withAttributes: [
                .font: font,
                .foregroundColor: color,
            ]
        )
    }

    private func highlightLine(_ line: Int) -> LineHighlight? {
        guard let highlight, highlight.lines.indices.contains(line) else { return nil }
        return highlight.lines[line]
    }

    private func font(for style: TokenStyle) -> NSFont {
        guard case let .inline(inlineStyle) = style else {
            return Self.codeFont
        }

        var traits: NSFontTraitMask = []
        if inlineStyle.isBold { traits.insert(.boldFontMask) }
        if inlineStyle.isItalic { traits.insert(.italicFontMask) }
        if traits.isEmpty { return Self.codeFont }
        return NSFontManager.shared.convert(Self.codeFont, toHaveTrait: traits)
    }

    private func substring(_ text: String, start: Int, end: Int) -> String {
        guard start < end, start < text.count else { return "" }
        let safeEnd = min(end, text.count)
        let startIndex = text.index(text.startIndex, offsetBy: start)
        let endIndex = text.index(text.startIndex, offsetBy: safeEnd)
        return String(text[startIndex..<endIndex])
    }

    private func blend(_ first: NSColor, with second: NSColor, ratio: CGFloat) -> NSColor {
        guard let a = first.usingColorSpace(.deviceRGB), let b = second.usingColorSpace(.deviceRGB) else {
            return first
        }
        return NSColor(
            calibratedRed: a.redComponent * ratio + b.redComponent * (1 - ratio),
            green: a.greenComponent * ratio + b.greenComponent * (1 - ratio),
            blue: a.blueComponent * ratio + b.blueComponent * (1 - ratio),
            alpha: a.alphaComponent * ratio + b.alphaComponent * (1 - ratio)
        )
    }

    private static func splitLines(_ source: String) -> [String] {
        source.isEmpty ? [] : source.components(separatedBy: "\n")
    }
}
