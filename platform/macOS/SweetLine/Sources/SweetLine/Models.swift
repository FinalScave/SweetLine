public struct HighlightConfig: Sendable, Equatable {
    public var showIndex: Bool
    public var inlineStyle: Bool

    public init(showIndex: Bool = false, inlineStyle: Bool = false) {
        self.showIndex = showIndex
        self.inlineStyle = inlineStyle
    }
}

public struct TextPosition: Sendable, Equatable {
    public let line: Int
    public let column: Int
    public let index: Int

    public init(line: Int, column: Int, index: Int = 0) {
        self.line = line
        self.column = column
        self.index = index
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

public struct TextLineInfo: Sendable, Equatable {
    public let line: Int
    public let startState: Int32
    public let startCharOffset: Int

    public init(line: Int, startState: Int32 = 0, startCharOffset: Int = 0) {
        self.line = line
        self.startState = startState
        self.startCharOffset = startCharOffset
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

public struct InlineStyle: Sendable, Equatable {
    public static let boldMask: Int32 = 1
    public static let italicMask: Int32 = 1 << 1
    public static let strikethroughMask: Int32 = 1 << 2

    public let foreground: Int32
    public let background: Int32
    public let isBold: Bool
    public let isItalic: Bool
    public let isStrikethrough: Bool

    public init(
        foreground: Int32,
        background: Int32,
        isBold: Bool,
        isItalic: Bool,
        isStrikethrough: Bool
    ) {
        self.foreground = foreground
        self.background = background
        self.isBold = isBold
        self.isItalic = isItalic
        self.isStrikethrough = isStrikethrough
    }

    public init(foreground: Int32, background: Int32, fontAttributes: Int32) {
        self.init(
            foreground: foreground,
            background: background,
            isBold: (fontAttributes & Self.boldMask) != 0,
            isItalic: (fontAttributes & Self.italicMask) != 0,
            isStrikethrough: (fontAttributes & Self.strikethroughMask) != 0
        )
    }
}

public enum TokenStyle: Sendable, Equatable {
    case styleId(Int32)
    case inline(InlineStyle)
}

public struct TokenSpan: Sendable, Equatable {
    public let range: TextRange
    public let style: TokenStyle

    public init(range: TextRange, style: TokenStyle) {
        self.range = range
        self.style = style
    }

    public init(range: TextRange, styleId: Int32) {
        self.init(range: range, style: .styleId(styleId))
    }

    public init(range: TextRange, inlineStyle: InlineStyle) {
        self.init(range: range, style: .inline(inlineStyle))
    }
}

public struct LineHighlight: Sendable, Equatable {
    public var spans: [TokenSpan]

    public init(spans: [TokenSpan] = []) {
        self.spans = spans
    }
}

public struct LineAnalyzeResult: Sendable, Equatable {
    public let highlight: LineHighlight
    public let endState: Int32
    public let charCount: Int

    public init(highlight: LineHighlight = LineHighlight(), endState: Int32 = 0, charCount: Int = 0) {
        self.highlight = highlight
        self.endState = endState
        self.charCount = charCount
    }
}

public struct DocumentHighlight: Sendable, Equatable {
    public var lines: [LineHighlight]

    public init(lines: [LineHighlight] = []) {
        self.lines = lines
    }
}

public struct DocumentHighlightSlice: Sendable, Equatable {
    public let startLine: Int
    public let totalLineCount: Int
    public var lines: [LineHighlight]

    public init(startLine: Int = 0, totalLineCount: Int = 0, lines: [LineHighlight] = []) {
        self.startLine = startLine
        self.totalLineCount = totalLineCount
        self.lines = lines
    }
}

public struct IndentGuideLine: Sendable, Equatable {
    public struct BranchPoint: Sendable, Equatable {
        public let line: Int
        public let column: Int

        public init(line: Int, column: Int) {
            self.line = line
            self.column = column
        }
    }

    public let column: Int
    public let startLine: Int
    public let endLine: Int
    public let nestingLevel: Int
    public let scopeRuleId: Int32
    public var branches: [BranchPoint]

    public init(
        column: Int,
        startLine: Int,
        endLine: Int,
        nestingLevel: Int,
        scopeRuleId: Int32,
        branches: [BranchPoint] = []
    ) {
        self.column = column
        self.startLine = startLine
        self.endLine = endLine
        self.nestingLevel = nestingLevel
        self.scopeRuleId = scopeRuleId
        self.branches = branches
    }
}

public struct LineScopeState: Sendable, Equatable {
    public let nestingLevel: Int
    public let scopeState: Int32
    public let scopeColumn: Int
    public let indentLevel: Int

    public init(nestingLevel: Int, scopeState: Int32, scopeColumn: Int, indentLevel: Int) {
        self.nestingLevel = nestingLevel
        self.scopeState = scopeState
        self.scopeColumn = scopeColumn
        self.indentLevel = indentLevel
    }
}

public struct IndentGuideResult: Sendable, Equatable {
    public var guideLines: [IndentGuideLine]
    public var lineStates: [LineScopeState]

    public init(guideLines: [IndentGuideLine] = [], lineStates: [LineScopeState] = []) {
        self.guideLines = guideLines
        self.lineStates = lineStates
    }
}
