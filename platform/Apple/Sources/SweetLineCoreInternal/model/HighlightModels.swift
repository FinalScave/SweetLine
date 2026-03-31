import Foundation

public struct FontAttributes: Sendable, Equatable {
    public let isBold: Bool
    public let isItalic: Bool
    public let isStrikethrough: Bool

    public init(isBold: Bool = false, isItalic: Bool = false, isStrikethrough: Bool = false) {
        self.isBold = isBold
        self.isItalic = isItalic
        self.isStrikethrough = isStrikethrough
    }
}

public struct InlineStyle: Sendable, Equatable {
    public let foregroundColor: Int32
    public let backgroundColor: Int32
    public let fontAttributes: FontAttributes

    public init(foregroundColor: Int32, backgroundColor: Int32, fontAttributes: FontAttributes) {
        self.foregroundColor = foregroundColor
        self.backgroundColor = backgroundColor
        self.fontAttributes = fontAttributes
    }
}

public struct TokenSpan: Sendable, Equatable {
    public let column: Int
    public let length: Int
    public let startIndex: Int?
    public let styleID: Int32?
    public let inlineStyle: InlineStyle?

    public init(column: Int, length: Int, startIndex: Int? = nil, styleID: Int32? = nil, inlineStyle: InlineStyle? = nil) {
        self.column = column
        self.length = length
        self.startIndex = startIndex
        self.styleID = styleID
        self.inlineStyle = inlineStyle
    }
}

public struct LineHighlight: Sendable, Equatable {
    public let spans: [TokenSpan]

    public init(spans: [TokenSpan] = []) {
        self.spans = spans
    }
}

public struct DocumentHighlight: Sendable, Equatable {
    public let lines: [LineHighlight]

    public init(lines: [LineHighlight] = []) {
        self.lines = lines
    }
}

public struct DocumentHighlightSlice: Sendable, Equatable {
    public let startLine: Int
    public let totalLineCount: Int
    public let lines: [LineHighlight]

    public init(startLine: Int = 0, totalLineCount: Int = 0, lines: [LineHighlight] = []) {
        self.startLine = startLine
        self.totalLineCount = totalLineCount
        self.lines = lines
    }
}

public struct TextLineInfo: Sendable, Equatable {
    public let line: Int
    public let startState: Int
    public let startCharOffset: Int

    public init(line: Int, startState: Int, startCharOffset: Int) {
        self.line = line
        self.startState = startState
        self.startCharOffset = startCharOffset
    }
}

public struct LineAnalyzeResult: Sendable, Equatable {
    public let highlight: LineHighlight
    public let endState: Int
    public let charCount: Int

    public init(highlight: LineHighlight = LineHighlight(), endState: Int = 0, charCount: Int = 0) {
        self.highlight = highlight
        self.endState = endState
        self.charCount = charCount
    }
}

public struct TextPosition: Sendable, Equatable {
    public let line: Int
    public let column: Int

    public init(line: Int, column: Int) {
        self.line = line
        self.column = column
    }
}

public struct TextRange: Sendable, Equatable {
    public let start: TextPosition
    public let end: TextPosition

    public init(start: TextPosition, end: TextPosition) {
        self.start = start
        self.end = end
    }
}

public struct LineRange: Sendable, Equatable {
    public let startLine: Int
    public let lineCount: Int

    public init(startLine: Int, lineCount: Int) {
        self.startLine = startLine
        self.lineCount = lineCount
    }
}

public struct IndentGuide: Sendable, Equatable {
    public let column: Int
    public let startLine: Int
    public let endLine: Int
    public let nestingLevel: Int

    public init(column: Int, startLine: Int, endLine: Int, nestingLevel: Int) {
        self.column = column
        self.startLine = startLine
        self.endLine = endLine
        self.nestingLevel = nestingLevel
    }
}

public struct IndentGuideLineState: Sendable, Equatable {
    public let nestingLevel: Int
    public let scopeState: Int
    public let scopeColumn: Int
    public let indentLevel: Int

    public init(nestingLevel: Int, scopeState: Int, scopeColumn: Int, indentLevel: Int) {
        self.nestingLevel = nestingLevel
        self.scopeState = scopeState
        self.scopeColumn = scopeColumn
        self.indentLevel = indentLevel
    }
}

public struct IndentGuideResult: Sendable, Equatable {
    public let guides: [IndentGuide]
    public let lineStates: [IndentGuideLineState]

    public init(guides: [IndentGuide] = [], lineStates: [IndentGuideLineState] = []) {
        self.guides = guides
        self.lineStates = lineStates
    }
}
