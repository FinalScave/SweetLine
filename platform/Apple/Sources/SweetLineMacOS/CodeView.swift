#if canImport(AppKit)
import AppKit

public final class CodeView: NSView {
    private let font = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)
    private let lineNumberFont = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    private let lineHeight: CGFloat = 20
    private let topPadding: CGFloat = 12
    private let bottomPadding: CGFloat = 12
    private let leftPadding: CGFloat = 10
    private let rightPadding: CGFloat = 20
    private let gutterSpacing: CGFloat = 10
    private let separatorWidth: CGFloat = 1

    private var currentModel: CodeRenderModel?
    private var errorMessage: String?

    public override var isFlipped: Bool {
        true
    }

    public override var intrinsicContentSize: NSSize {
        guard let model = currentModel else {
            return NSSize(width: 640, height: 480)
        }

        let lines = model.sourceText.components(separatedBy: .newlines)
        let layout = metrics(for: model, lines: lines)
        let size = CodeLayout.preferredContentSize(
            lines: lines,
            lineHeight: lineHeight,
            characterWidth: layout.characterWidth,
            gutterWidth: layout.textOriginX
        )

        return NSSize(width: size.width + rightPadding, height: max(size.height + topPadding + bottomPadding, 480))
    }

    public func apply(model: CodeRenderModel) {
        currentModel = model
        errorMessage = nil
        frame.size = intrinsicContentSize
        invalidateIntrinsicContentSize()
        needsDisplay = true
    }

    public func applyError(_ message: String) {
        currentModel = nil
        errorMessage = message
        frame.size = intrinsicContentSize
        invalidateIntrinsicContentSize()
        needsDisplay = true
    }

    public override func draw(_ dirtyRect: NSRect) {
        if let model = currentModel {
            drawCode(model: model, dirtyRect: dirtyRect)
            return
        }

        drawError(message: errorMessage ?? "Unknown error", dirtyRect: dirtyRect)
    }

    private func drawCode(model: CodeRenderModel, dirtyRect: NSRect) {
        let lines = model.sourceText.components(separatedBy: .newlines)
        let metrics = metrics(for: model, lines: lines)

        color(from: model.theme.backgroundColorARGB).setFill()
        dirtyRect.fill()

        drawGutter(metrics: metrics, theme: model.theme)
        drawIndentGuides(model: model, metrics: metrics)
        drawTextLines(model: model, lines: lines, metrics: metrics)
    }

    private func drawError(message: String, dirtyRect: NSRect) {
        NSColor.windowBackgroundColor.setFill()
        dirtyRect.fill()

        let attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.monospacedSystemFont(ofSize: 13, weight: .regular),
            .foregroundColor: NSColor.secondaryLabelColor,
        ]

        let rect = NSRect(x: 24, y: 24, width: bounds.width - 48, height: bounds.height - 48)
        message.draw(in: rect, withAttributes: attributes)
    }

    private func drawGutter(metrics: RenderMetrics, theme: HighlightTheme) {
        let gutterRect = NSRect(x: 0, y: 0, width: metrics.textOriginX - gutterSpacing, height: bounds.height)
        color(from: blend(theme.backgroundColorARGB, with: Int32(bitPattern: 0xFFFFFFFF), amount: 0.05)).setFill()
        gutterRect.fill()

        let separatorRect = NSRect(x: metrics.textOriginX - gutterSpacing - separatorWidth, y: 0, width: separatorWidth, height: bounds.height)
        color(from: blend(theme.textColorARGB, with: theme.backgroundColorARGB, amount: 0.7)).setFill()
        separatorRect.fill()
    }

    private func drawIndentGuides(model: CodeRenderModel, metrics: RenderMetrics) {
        let guideColor = color(from: blend(model.theme.textColorARGB, with: model.theme.backgroundColorARGB, amount: 0.75))
        guideColor.setStroke()

        let path = NSBezierPath()
        path.lineWidth = 1

        for guide in model.indentGuides.guides {
            let innerStart = guide.startLine + 1
            let innerEnd = guide.endLine - 1
            if innerStart > innerEnd {
                continue
            }
            let x = metrics.textOriginX + CGFloat(guide.column) * metrics.characterWidth + metrics.characterWidth * 0.5
            let startY = topPadding + CGFloat(innerStart) * lineHeight
            let endY = topPadding + CGFloat(innerEnd + 1) * lineHeight
            path.move(to: NSPoint(x: x, y: startY))
            path.line(to: NSPoint(x: x, y: endY))
        }

        path.stroke()
    }

    private func drawTextLines(model: CodeRenderModel, lines: [String], metrics: RenderMetrics) {
        for (lineIndex, lineText) in lines.enumerated() {
            let lineY = topPadding + CGFloat(lineIndex) * lineHeight + 2
            drawLineNumber(lineIndex: lineIndex, y: lineY, metrics: metrics, theme: model.theme)

            let lineHighlight = lineIndex < model.highlight.lines.count ? model.highlight.lines[lineIndex] : LineHighlight()
            let segments = CodeLayout.makeSegments(lineText: lineText, spans: lineHighlight.spans)
            drawSegments(segments, x: metrics.textOriginX, y: lineY, theme: model.theme)
        }
    }

    private func drawLineNumber(lineIndex: Int, y: CGFloat, metrics: RenderMetrics, theme: HighlightTheme) {
        let text = "\(lineIndex + 1)" as NSString
        let colorValue = blend(theme.textColorARGB, with: theme.backgroundColorARGB, amount: 0.45)
        let attributes: [NSAttributedString.Key: Any] = [
            .font: lineNumberFont,
            .foregroundColor: color(from: colorValue),
        ]

        let size = text.size(withAttributes: attributes)
        let x = metrics.textOriginX - gutterSpacing - separatorWidth - size.width - 8
        text.draw(at: NSPoint(x: x, y: y + 1), withAttributes: attributes)
    }

    private func drawSegments(_ segments: [CodeSegment], x: CGFloat, y: CGFloat, theme: HighlightTheme) {
        var currentX = x

        for segment in segments {
            let colorValue = segment.foregroundColorARGB(theme: theme)
            let attributes: [NSAttributedString.Key: Any] = [
                .font: font,
                .foregroundColor: color(from: colorValue),
            ]

            let text = segment.text as NSString
            text.draw(at: NSPoint(x: currentX, y: y), withAttributes: attributes)
            currentX += text.size(withAttributes: attributes).width
        }
    }

    private func metrics(for model: CodeRenderModel, lines: [String]) -> RenderMetrics {
        let characterWidth = characterAdvance(for: font)
        let digitWidth = characterAdvance(for: lineNumberFont)
        let gutterWidth = CodeLayout.gutterWidth(lineCount: max(lines.count, 1), digitWidth: digitWidth)
        let textOriginX = gutterWidth + gutterSpacing + leftPadding
        return RenderMetrics(characterWidth: characterWidth, gutterWidth: gutterWidth, textOriginX: textOriginX)
    }

    private func characterAdvance(for font: NSFont) -> CGFloat {
        let attributes: [NSAttributedString.Key: Any] = [.font: font]
        return max(("M" as NSString).size(withAttributes: attributes).width, 7)
    }

    private func color(from argb: Int32) -> NSColor {
        let value = UInt32(bitPattern: argb)
        let alpha = CGFloat((value >> 24) & 0xFF) / 255
        let red = CGFloat((value >> 16) & 0xFF) / 255
        let green = CGFloat((value >> 8) & 0xFF) / 255
        let blue = CGFloat(value & 0xFF) / 255
        return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
    }

    private func blend(_ base: Int32, with overlay: Int32, amount: CGFloat) -> Int32 {
        let baseValue = UInt32(bitPattern: base)
        let overlayValue = UInt32(bitPattern: overlay)
        let clamped = min(max(amount, 0), 1)

        func channel(_ value: UInt32, shift: UInt32) -> CGFloat {
            CGFloat((value >> shift) & 0xFF)
        }

        let alpha = channel(baseValue, shift: 24)
        let red = channel(baseValue, shift: 16) * (1 - clamped) + channel(overlayValue, shift: 16) * clamped
        let green = channel(baseValue, shift: 8) * (1 - clamped) + channel(overlayValue, shift: 8) * clamped
        let blue = channel(baseValue, shift: 0) * (1 - clamped) + channel(overlayValue, shift: 0) * clamped

        let composed = (UInt32(alpha.rounded()) << 24)
            | (UInt32(red.rounded()) << 16)
            | (UInt32(green.rounded()) << 8)
            | UInt32(blue.rounded())
        return Int32(bitPattern: composed)
    }
}

private struct RenderMetrics {
    let characterWidth: CGFloat
    let gutterWidth: CGFloat
    let textOriginX: CGFloat
}
#endif
