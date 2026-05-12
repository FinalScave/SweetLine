import SweetLine
import SwiftUI
import UIKit

struct HighlightedCodeView: UIViewRepresentable {
    let source: String
    let highlight: DocumentHighlight?
    let indentGuides: IndentGuideResult?
    let theme: HighlightTheme
    let onTextChange: (NSRange, String) -> Void

    func makeCoordinator() -> Coordinator {
        Coordinator(onTextChange: onTextChange)
    }

    func makeUIView(context: Context) -> UITextView {
        let textView = UITextView()
        textView.delegate = context.coordinator
        textView.isEditable = true
        textView.isSelectable = true
        textView.isScrollEnabled = true
        textView.alwaysBounceVertical = true
        textView.autocorrectionType = .no
        textView.autocapitalizationType = .none
        textView.smartQuotesType = .no
        textView.smartDashesType = .no
        textView.smartInsertDeleteType = .no
        textView.keyboardDismissMode = .interactive
        textView.textContainerInset = UIEdgeInsets(top: 10, left: 10, bottom: 14, right: 10)
        textView.textContainer.lineFragmentPadding = 0
        return textView
    }

    func updateUIView(_ textView: UITextView, context: Context) {
        context.coordinator.onTextChange = onTextChange
        context.coordinator.source = source

        textView.backgroundColor = theme.backgroundUIColor
        textView.indicatorStyle = theme.isDark ? .white : .black
        textView.keyboardAppearance = theme.isDark ? .dark : .light
        textView.typingAttributes = Self.baseAttributes(theme: theme, font: Self.codeFont)

        let selectedRange = textView.selectedRange
        let contentOffset = textView.contentOffset
        let attributedText = Self.attributedString(source: source, highlight: highlight, theme: theme)

        context.coordinator.isApplyingHighlight = true
        textView.attributedText = attributedText
        textView.selectedRange = Self.clamped(range: selectedRange, length: attributedText.length)
        textView.setContentOffset(contentOffset, animated: false)
        context.coordinator.isApplyingHighlight = false
    }

    final class Coordinator: NSObject, UITextViewDelegate {
        var onTextChange: (NSRange, String) -> Void
        var source = ""
        var isApplyingHighlight = false
        private var pendingEdit: (range: NSRange, replacement: String)?

        init(onTextChange: @escaping (NSRange, String) -> Void) {
            self.onTextChange = onTextChange
        }

        func textView(_ textView: UITextView, shouldChangeTextIn range: NSRange, replacementText text: String) -> Bool {
            guard !isApplyingHighlight else { return true }
            pendingEdit = (range, text)
            return true
        }

        func textViewDidChange(_ textView: UITextView) {
            guard !isApplyingHighlight else { return }
            if let edit = pendingEdit {
                pendingEdit = nil
                onTextChange(edit.range, edit.replacement)
            } else {
                onTextChange(NSRange(location: 0, length: (source as NSString).length), textView.text ?? "")
            }
        }
    }

    private static let codeFont = UIFont.monospacedSystemFont(ofSize: 14, weight: .regular)

    private static func attributedString(source: String, highlight: DocumentHighlight?, theme: HighlightTheme) -> NSAttributedString {
        let attributed = NSMutableAttributedString(
            string: source,
            attributes: baseAttributes(theme: theme, font: codeFont)
        )

        guard let highlight else { return attributed }
        let lines = source.components(separatedBy: "\n")
        let lineStarts = utf16LineStarts(in: lines)

        for lineIndex in highlight.lines.indices {
            guard lineIndex < lines.count else { continue }
            for span in highlight.lines[lineIndex].spans {
                guard let nsRange = nsRange(for: span, lines: lines, lineStarts: lineStarts) else { continue }
                attributed.addAttributes(attributes(for: span.style, theme: theme), range: nsRange)
            }
        }

        return attributed
    }

    private static func baseAttributes(theme: HighlightTheme, font: UIFont) -> [NSAttributedString.Key: Any] {
        let paragraph = NSMutableParagraphStyle()
        paragraph.lineBreakMode = .byWordWrapping
        paragraph.lineSpacing = 1

        return [
            .font: font,
            .foregroundColor: theme.textUIColor,
            .paragraphStyle: paragraph,
        ]
    }

    private static func attributes(for style: TokenStyle, theme: HighlightTheme) -> [NSAttributedString.Key: Any] {
        var attributes: [NSAttributedString.Key: Any] = [
            .foregroundColor: theme.foreground(for: style),
            .font: font(for: style),
        ]

        if case let .inline(inlineStyle) = style {
            if inlineStyle.background != 0 {
                attributes[.backgroundColor] = UIColor(argb: UInt32(bitPattern: inlineStyle.background))
            }
            if inlineStyle.isStrikethrough {
                attributes[.strikethroughStyle] = NSUnderlineStyle.single.rawValue
            }
        }

        return attributes
    }

    private static func font(for style: TokenStyle) -> UIFont {
        guard case let .inline(inlineStyle) = style else {
            return codeFont
        }

        var traits: UIFontDescriptor.SymbolicTraits = []
        if inlineStyle.isBold { traits.insert(.traitBold) }
        if inlineStyle.isItalic { traits.insert(.traitItalic) }
        guard !traits.isEmpty, let descriptor = codeFont.fontDescriptor.withSymbolicTraits(traits) else {
            return codeFont
        }
        return UIFont(descriptor: descriptor, size: codeFont.pointSize)
    }

    private static func nsRange(for span: TokenSpan, lines: [String], lineStarts: [Int]) -> NSRange? {
        let line = span.range.start.line
        guard lines.indices.contains(line), lineStarts.indices.contains(line) else { return nil }

        let lineText = lines[line]
        let startColumn = max(0, min(span.range.start.column, lineText.count))
        let endColumn = max(startColumn, min(span.range.end.column, lineText.count))
        guard let startOffset = utf16Offset(in: lineText, column: startColumn),
              let endOffset = utf16Offset(in: lineText, column: endColumn) else {
            return nil
        }

        return NSRange(location: lineStarts[line] + startOffset, length: endOffset - startOffset)
    }

    private static func utf16LineStarts(in lines: [String]) -> [Int] {
        var starts: [Int] = []
        starts.reserveCapacity(lines.count)
        var offset = 0
        for line in lines {
            starts.append(offset)
            offset += line.utf16.count + 1
        }
        return starts
    }

    private static func utf16Offset(in line: String, column: Int) -> Int? {
        let index = line.index(line.startIndex, offsetBy: min(column, line.count))
        guard let utf16Index = index.samePosition(in: line.utf16) else { return nil }
        return line.utf16.distance(from: line.utf16.startIndex, to: utf16Index)
    }

    private static func clamped(range: NSRange, length: Int) -> NSRange {
        let location = min(max(range.location, 0), length)
        let upperBound = min(max(range.location + range.length, location), length)
        return NSRange(location: location, length: upperBound - location)
    }
}

private extension UIColor {
    convenience init(argb: UInt32) {
        let alpha = CGFloat((argb >> 24) & 0xFF) / 255.0
        let red = CGFloat((argb >> 16) & 0xFF) / 255.0
        let green = CGFloat((argb >> 8) & 0xFF) / 255.0
        let blue = CGFloat(argb & 0xFF) / 255.0
        self.init(red: red, green: green, blue: blue, alpha: alpha)
    }
}
