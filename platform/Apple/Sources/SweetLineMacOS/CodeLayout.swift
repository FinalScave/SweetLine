import CoreGraphics

public struct HighlightTheme: Equatable {
    public let name: String
    public let backgroundColorARGB: Int32
    public let textColorARGB: Int32
    public let colorMap: [Int32: Int32]

    public init(name: String, backgroundColorARGB: Int32, textColorARGB: Int32, colorMap: [Int32: Int32]) {
        self.name = name
        self.backgroundColorARGB = backgroundColorARGB
        self.textColorARGB = textColorARGB
        self.colorMap = colorMap
    }

    public func color(for styleID: Int32) -> Int32 {
        colorMap[styleID] ?? textColorARGB
    }
}

public struct CodeRenderModel: Equatable {
    public let sourceText: String
    public let highlight: DocumentHighlight
    public let indentGuides: IndentGuideResult
    public let theme: HighlightTheme

    public init(sourceText: String, highlight: DocumentHighlight, indentGuides: IndentGuideResult, theme: HighlightTheme) {
        self.sourceText = sourceText
        self.highlight = highlight
        self.indentGuides = indentGuides
        self.theme = theme
    }
}

public struct CodeSegment: Equatable {
    public let text: String
    public let styleID: Int32?
    public let inlineStyle: InlineStyle?

    public init(text: String, styleID: Int32?, inlineStyle: InlineStyle? = nil) {
        self.text = text
        self.styleID = styleID
        self.inlineStyle = inlineStyle
    }

    public func foregroundColorARGB(theme: HighlightTheme) -> Int32 {
        if let inlineColor = inlineStyle?.foregroundColor {
            return inlineColor
        }

        return styleID.map(theme.color(for:)) ?? theme.textColorARGB
    }
}

public enum CodeLayout {
    private static let lineNumberPadding: CGFloat = 12
    private static let codeLeftPadding: CGFloat = 8
    private static let trailingPadding: CGFloat = 20
    private static let verticalPadding: CGFloat = 10

    public static func gutterWidth(lineCount: Int, digitWidth: CGFloat) -> CGFloat {
        let safeLineCount = max(lineCount, 1)
        let digitCount = String(safeLineCount).count
        return CGFloat(digitCount) * digitWidth + lineNumberPadding * 2
    }

    public static func makeSegments(lineText: String, spans: [TokenSpan]) -> [CodeSegment] {
        guard !lineText.isEmpty else {
            return []
        }

        var segments: [CodeSegment] = []
        var cursor = 0

        for span in spans {
            let start = min(max(span.column, 0), lineText.count)
            let end = min(max(span.column + span.length, start), lineText.count)

            if start > cursor {
                segments.append(CodeSegment(text: slice(lineText, from: cursor, to: start), styleID: nil))
            }

            if end > start {
                segments.append(CodeSegment(text: slice(lineText, from: start, to: end), styleID: span.styleID, inlineStyle: span.inlineStyle))
            }

            cursor = max(cursor, end)
        }

        if cursor < lineText.count {
            segments.append(CodeSegment(text: slice(lineText, from: cursor, to: lineText.count), styleID: nil))
        }

        return segments
    }

    public static func preferredContentSize(
        lines: [String],
        lineHeight: CGFloat,
        characterWidth: CGFloat,
        gutterWidth: CGFloat
    ) -> CGSize {
        let safeLines = lines.isEmpty ? [""] : lines
        let longestLineLength = safeLines.map(\.count).max() ?? 0
        let width = gutterWidth + codeLeftPadding + CGFloat(longestLineLength) * characterWidth + trailingPadding
        let height = CGFloat(safeLines.count) * lineHeight + verticalPadding
        return CGSize(width: width, height: height)
    }

    private static func slice(_ text: String, from start: Int, to end: Int) -> String {
        let lowerBound = text.index(text.startIndex, offsetBy: start)
        let upperBound = text.index(text.startIndex, offsetBy: end)
        return String(text[lowerBound..<upperBound])
    }
}
