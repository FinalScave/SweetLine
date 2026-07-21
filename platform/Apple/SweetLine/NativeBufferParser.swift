enum NativeBufferParser {
    static func readDocumentHighlight(_ buffer: UnsafeMutablePointer<Int32>) -> DocumentHighlight {
        let flags = buffer[0]
        let stride = Int(buffer[1])
        let lineCount = nonNegative(buffer[2])
        let hasStartIndex = flagsHasStartIndex(flags)
        let inlineStyle = flagsUsesInlineStyle(flags)
        guard isValidSpanStride(stride, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle) else {
            return DocumentHighlight()
        }

        var index = 3
        var lines: [LineHighlight] = []
        lines.reserveCapacity(lineCount)

        for line in 0..<lineCount {
            let spanCount = nonNegative(buffer[index])
            index += 1
            var spans: [TokenSpan] = []
            spans.reserveCapacity(spanCount)

            for _ in 0..<spanCount {
                spans.append(readTokenSpan(buffer, index: &index, line: line, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle))
            }

            lines.append(LineHighlight(spans: spans))
        }

        return DocumentHighlight(lines: lines)
    }

    static func readDocumentHighlightSlice(_ buffer: UnsafeMutablePointer<Int32>) -> DocumentHighlightSlice {
        let flags = buffer[0]
        let stride = Int(buffer[1])
        let startLine = Int(buffer[2])
        let totalLineCount = Int(buffer[3])
        let lineCount = nonNegative(buffer[4])
        let hasStartIndex = flagsHasStartIndex(flags)
        let inlineStyle = flagsUsesInlineStyle(flags)
        guard isValidSpanStride(stride, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle) else {
            return DocumentHighlightSlice(startLine: startLine, totalLineCount: totalLineCount)
        }

        var index = 5
        var lines: [LineHighlight] = []
        lines.reserveCapacity(lineCount)

        for offset in 0..<lineCount {
            let spanCount = nonNegative(buffer[index])
            index += 1
            var spans: [TokenSpan] = []
            spans.reserveCapacity(spanCount)

            for _ in 0..<spanCount {
                spans.append(readTokenSpan(buffer, index: &index, line: startLine + offset, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle))
            }

            lines.append(LineHighlight(spans: spans))
        }

        return DocumentHighlightSlice(startLine: startLine, totalLineCount: totalLineCount, lines: lines)
    }

    static func readLineAnalyzeResult(_ buffer: UnsafeMutablePointer<Int32>, lineNumber: Int = 0) -> LineAnalyzeResult {
        let flags = buffer[0]
        let stride = Int(buffer[1])
        let spanCount = nonNegative(buffer[2])
        let endState = buffer[3]
        let charCount = nonNegative(buffer[4])
        let hasStartIndex = flagsHasStartIndex(flags)
        let inlineStyle = flagsUsesInlineStyle(flags)
        guard isValidSpanStride(stride, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle) else {
            return LineAnalyzeResult(endState: endState, charCount: charCount)
        }

        var index = 5
        var spans: [TokenSpan] = []
        spans.reserveCapacity(spanCount)
        for _ in 0..<spanCount {
            spans.append(readTokenSpan(buffer, index: &index, line: lineNumber, hasStartIndex: hasStartIndex, inlineStyle: inlineStyle))
        }

        return LineAnalyzeResult(highlight: LineHighlight(spans: spans), endState: endState, charCount: charCount)
    }

    static func readIndentGuideResult(_ buffer: UnsafeMutablePointer<Int32>) -> IndentGuideResult {
        let startLine = Int(buffer[0])
        let lineStateCount = nonNegative(buffer[1])
        let guideCount = nonNegative(buffer[2])
        var index = 3
        var guideLines: [IndentGuideLine] = []
        guideLines.reserveCapacity(guideCount)

        for _ in 0..<guideCount {
            let column = Int(buffer[index]); index += 1
            let startLine = Int(buffer[index]); index += 1
            let endLine = Int(buffer[index]); index += 1
            let flags = buffer[index]; index += 1
            let continuesBefore = (flags & 1) != 0
            let continuesAfter = (flags & (1 << 1)) != 0
            let branchCount = nonNegative(buffer[index]); index += 1

            var branches: [IndentGuideLine.BranchPoint] = []
            branches.reserveCapacity(branchCount)
            for _ in 0..<branchCount {
                let line = Int(buffer[index]); index += 1
                let column = Int(buffer[index]); index += 1
                branches.append(IndentGuideLine.BranchPoint(line: line, column: column))
            }

            guideLines.append(IndentGuideLine(
                column: column,
                startLine: startLine,
                endLine: endLine,
                continuesBefore: continuesBefore,
                continuesAfter: continuesAfter,
                branches: branches
            ))
        }

        var lineStates: [LineScopeState] = []
        lineStates.reserveCapacity(lineStateCount)
        for _ in 0..<lineStateCount {
            let nestingLevel = Int(buffer[index]); index += 1
            let scopeState = buffer[index]; index += 1
            let scopeColumn = Int(buffer[index]); index += 1
            let indentLevel = Int(buffer[index]); index += 1
            lineStates.append(LineScopeState(
                nestingLevel: nestingLevel,
                scopeState: scopeState,
                scopeColumn: scopeColumn,
                indentLevel: indentLevel
            ))
        }

        return IndentGuideResult(startLine: startLine, guideLines: guideLines, lineStates: lineStates)
    }

    static func readBracketPairResult(_ buffer: UnsafeMutablePointer<Int32>) -> BracketPairResult {
        let flags = buffer[0]
        let stride = Int(buffer[1])
        let lineCount = nonNegative(buffer[2])
        let hasStartIndex = flagsHasStartIndex(flags)
        guard isValidBracketTokenStride(stride, hasStartIndex: hasStartIndex) else {
            return BracketPairResult()
        }

        var index = 3
        let lines = readBracketLines(buffer, index: &index, startLine: 0, lineCount: lineCount, hasStartIndex: hasStartIndex)
        return BracketPairResult(startLine: 0, totalLineCount: lineCount, lines: lines)
    }

    static func readBracketPairResultSlice(_ buffer: UnsafeMutablePointer<Int32>) -> BracketPairResult {
        let flags = buffer[0]
        let stride = Int(buffer[1])
        let startLine = Int(buffer[2])
        let totalLineCount = Int(buffer[3])
        let lineCount = nonNegative(buffer[4])
        let hasStartIndex = flagsHasStartIndex(flags)
        guard isValidBracketTokenStride(stride, hasStartIndex: hasStartIndex) else {
            return BracketPairResult(startLine: startLine, totalLineCount: totalLineCount)
        }

        var index = 5
        let lines = readBracketLines(buffer, index: &index, startLine: startLine, lineCount: lineCount, hasStartIndex: hasStartIndex)
        return BracketPairResult(startLine: startLine, totalLineCount: totalLineCount, lines: lines)
    }

    private static func readTokenSpan(
        _ buffer: UnsafeMutablePointer<Int32>,
        index: inout Int,
        line: Int,
        hasStartIndex: Bool,
        inlineStyle: Bool
    ) -> TokenSpan {
        let startColumn = Int(buffer[index]); index += 1
        let length = Int(buffer[index]); index += 1
        let startIndex: Int
        if hasStartIndex {
            startIndex = Int(buffer[index])
            index += 1
        } else {
            startIndex = 0
        }

        let range = TextRange(
            start: TextPosition(line: line, column: startColumn, index: startIndex),
            end: TextPosition(line: line, column: startColumn + length, index: hasStartIndex ? startIndex + length : 0)
        )

        if inlineStyle {
            let foreground = buffer[index]; index += 1
            let background = buffer[index]; index += 1
            let fontAttributes = buffer[index]; index += 1
            return TokenSpan(range: range, inlineStyle: InlineStyle(
                foreground: foreground,
                background: background,
                fontAttributes: fontAttributes
            ))
        }

        let styleId = buffer[index]
        index += 1
        return TokenSpan(range: range, styleId: styleId)
    }

    private static func readBracketLines(
        _ buffer: UnsafeMutablePointer<Int32>,
        index: inout Int,
        startLine: Int,
        lineCount: Int,
        hasStartIndex: Bool
    ) -> [LineBracketPairs] {
        var lines: [LineBracketPairs] = []
        lines.reserveCapacity(lineCount)

        for offset in 0..<lineCount {
            let tokenCount = nonNegative(buffer[index])
            index += 1
            var tokens: [BracketToken] = []
            tokens.reserveCapacity(tokenCount)

            for _ in 0..<tokenCount {
                tokens.append(readBracketToken(buffer, index: &index, line: startLine + offset, hasStartIndex: hasStartIndex))
            }

            lines.append(LineBracketPairs(tokens: tokens))
        }

        return lines
    }

    private static func readBracketToken(
        _ buffer: UnsafeMutablePointer<Int32>,
        index: inout Int,
        line: Int,
        hasStartIndex: Bool
    ) -> BracketToken {
        let column = Int(buffer[index]); index += 1
        let length = Int(buffer[index]); index += 1
        let tokenStartIndex: Int
        if hasStartIndex {
            tokenStartIndex = Int(buffer[index])
            index += 1
        } else {
            tokenStartIndex = 0
        }

        let depth = Int(buffer[index]); index += 1
        let kind = BracketTokenKind(nativeValue: buffer[index]); index += 1
        let matchState = BracketMatchState(nativeValue: buffer[index]); index += 1
        let partnerLine = Int(buffer[index]); index += 1
        let partnerColumn = Int(buffer[index]); index += 1
        let partnerLength = Int(buffer[index]); index += 1
        let partnerStartIndex: Int
        if hasStartIndex {
            partnerStartIndex = Int(buffer[index])
            index += 1
        } else {
            partnerStartIndex = 0
        }

        let range = TextRange(
            start: TextPosition(line: line, column: column, index: tokenStartIndex),
            end: TextPosition(line: line, column: column + length, index: hasStartIndex ? tokenStartIndex + length : 0)
        )
        let partnerRange: TextRange?
        if partnerLine >= 0, partnerColumn >= 0, partnerLength >= 0 {
            partnerRange = TextRange(
                start: TextPosition(line: partnerLine, column: partnerColumn, index: partnerStartIndex),
                end: TextPosition(
                    line: partnerLine,
                    column: partnerColumn + partnerLength,
                    index: hasStartIndex ? partnerStartIndex + partnerLength : 0
                )
            )
        } else {
            partnerRange = nil
        }

        return BracketToken(range: range, depth: depth, kind: kind, matchState: matchState, partnerRange: partnerRange)
    }

    private static func isValidSpanStride(_ stride: Int, hasStartIndex: Bool, inlineStyle: Bool) -> Bool {
        let expected = 2 + (hasStartIndex ? 1 : 0) + (inlineStyle ? 3 : 1)
        return stride == expected
    }

    private static func isValidBracketTokenStride(_ stride: Int, hasStartIndex: Bool) -> Bool {
        let expected = 8 + (hasStartIndex ? 2 : 0)
        return stride == expected
    }

    private static func flagsHasStartIndex(_ flags: Int32) -> Bool {
        (flags & 1) != 0
    }

    private static func flagsUsesInlineStyle(_ flags: Int32) -> Bool {
        (flags & (1 << 1)) != 0
    }

    private static func nonNegative(_ value: Int32) -> Int {
        max(Int(value), 0)
    }
}
