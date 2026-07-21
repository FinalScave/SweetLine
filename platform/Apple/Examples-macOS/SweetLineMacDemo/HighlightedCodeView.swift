import AppKit
import SwiftUI
import SweetLine

struct HighlightedCodeView: NSViewRepresentable {
    let source: String
    let highlight: DocumentHighlight?
    let indentGuides: IndentGuideResult?
    let bracketPairs: BracketPairResult?
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
        nsView.bracketPairs = bracketPairs
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
    private static let bracketPalette: [NSColor] = [
        nsColor(argb: 0xFF7DD3FC),
        nsColor(argb: 0xFFF9A8D4),
        nsColor(argb: 0xFFFDE047),
        nsColor(argb: 0xFF86EFAC),
        nsColor(argb: 0xFFC4B5FD),
        nsColor(argb: 0xFFFDBA74),
    ]

    var source = "" { didSet { needsDisplay = true } }
    var highlight: DocumentHighlight? { didSet { needsDisplay = true } }
    var indentGuides: IndentGuideResult? { didSet { needsDisplay = true } }
    var bracketPairs: BracketPairResult? { didSet { needsDisplay = true } }
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

            drawCodeLine(
                lines[lineIndex],
                spans: highlightLine(lineIndex)?.spans ?? [],
                bracketTokens: bracketLine(lineIndex)?.tokens ?? [],
                x: codeX,
                baseline: baseline
            )
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

    private func drawCodeLine(_ lineText: String, spans: [TokenSpan], bracketTokens: [BracketToken], x: CGFloat, baseline: CGFloat) {
        let attributed = NSMutableAttributedString(
            string: lineText,
            attributes: [
                .font: Self.codeFont,
                .foregroundColor: theme.textNSColor,
            ]
        )

        for span in spans {
            guard let range = nsRange(in: lineText, start: span.range.start.column, end: span.range.end.column) else { continue }
            attributed.addAttributes(attributes(for: span.style), range: range)
        }

        for token in bracketTokens {
            guard let range = nsRange(in: lineText, start: token.range.start.column, end: token.range.end.column) else { continue }
            attributed.addAttribute(.foregroundColor, value: bracketColor(for: token), range: range)
        }

        attributed.draw(at: CGPoint(x: x, y: baseline - Self.codeFont.ascender))
    }

    private func attributes(for style: TokenStyle) -> [NSAttributedString.Key: Any] {
        var attributes: [NSAttributedString.Key: Any] = [
            .foregroundColor: theme.foreground(for: style),
            .font: font(for: style),
        ]

        if case let .inline(inlineStyle) = style {
            if inlineStyle.background != 0 {
                attributes[.backgroundColor] = Self.nsColor(argb: UInt32(bitPattern: inlineStyle.background))
            }
            if inlineStyle.isStrikethrough {
                attributes[.strikethroughStyle] = NSUnderlineStyle.single.rawValue
            }
        }

        return attributes
    }

    private func nsRange(in lineText: String, start: Int, end: Int) -> NSRange? {
        let scalarCount = lineText.unicodeScalars.count
        let startColumn = max(0, min(start, scalarCount))
        let endColumn = max(startColumn, min(end, scalarCount))
        guard endColumn > startColumn else { return nil }

        var currentScalarOffset = 0
        var utf16Offset = 0
        var startOffset: Int?
        var endOffset: Int?

        for scalar in lineText.unicodeScalars {
            if currentScalarOffset == startColumn {
                startOffset = utf16Offset
            }
            if currentScalarOffset == endColumn {
                endOffset = utf16Offset
                break
            }
            utf16Offset += scalar.utf16.count
            currentScalarOffset += 1
        }

        if currentScalarOffset == startColumn {
            startOffset = utf16Offset
        }
        if currentScalarOffset == endColumn {
            endOffset = utf16Offset
        }

        guard let startOffset, let endOffset, endOffset > startOffset else { return nil }
        return NSRange(location: startOffset, length: endOffset - startOffset)
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

    private func bracketLine(_ line: Int) -> LineBracketPairs? {
        guard let bracketPairs else { return nil }
        let index = line - bracketPairs.startLine
        guard bracketPairs.lines.indices.contains(index) else { return nil }
        return bracketPairs.lines[index]
    }

    private func bracketColor(for token: BracketToken) -> NSColor {
        if token.matchState == .unmatched {
            return Self.nsColor(argb: 0xFFFF6B6B)
        }
        let index = ((token.depth % Self.bracketPalette.count) + Self.bracketPalette.count) % Self.bracketPalette.count
        let color = Self.bracketPalette[index]
        return token.matchState == .unknown ? color.withAlphaComponent(0.68) : color
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

    private static func nsColor(argb: UInt32) -> NSColor {
        let alpha = CGFloat((argb >> 24) & 0xFF) / 255.0
        let red = CGFloat((argb >> 16) & 0xFF) / 255.0
        let green = CGFloat((argb >> 8) & 0xFF) / 255.0
        let blue = CGFloat(argb & 0xFF) / 255.0
        return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
    }

    private static func splitLines(_ source: String) -> [String] {
        source.isEmpty ? [] : source.components(separatedBy: "\n")
    }
}
