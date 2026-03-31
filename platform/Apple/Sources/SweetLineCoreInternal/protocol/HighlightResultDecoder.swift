import Foundation

enum HighlightResultDecoder {
    static func decodeDocumentHighlight(from buffer: UnsafeMutablePointer<Int32>?) throws -> DocumentHighlight {
        guard let buffer else {
            return DocumentHighlight()
        }

        let flags = Int(buffer[0])
        let stride = max(Int(buffer[1]), 0)
        let lineCount = max(Int(buffer[2]), 0)
        let hasStartIndex = flagsHasStartIndex(flags)
        let usesInlineStyle = flagsUsesInlineStyle(flags)

        guard isValidSpanStride(stride: stride, hasStartIndex: hasStartIndex, usesInlineStyle: usesInlineStyle) else {
            throw SweetLineError.invalidResult("Document highlight stride is invalid.")
        }

        var cursor = 3
        var lines: [LineHighlight] = []
        lines.reserveCapacity(lineCount)

        for _ in 0..<lineCount {
            let spanCount = max(Int(buffer[cursor]), 0)
            cursor += 1
            var spans: [TokenSpan] = []
            spans.reserveCapacity(spanCount)

            for _ in 0..<spanCount {
                let column = Int(buffer[cursor])
                let length = Int(buffer[cursor + 1])
                cursor += 2

                let startIndex: Int?
                if hasStartIndex {
                    startIndex = Int(buffer[cursor])
                    cursor += 1
                } else {
                    startIndex = nil
                }

                if usesInlineStyle {
                    let foregroundColor = buffer[cursor]
                    let backgroundColor = buffer[cursor + 1]
                    let styleFlags = Int(buffer[cursor + 2])
                    cursor += 3
                    let attributes = FontAttributes(
                        isBold: (styleFlags & 1) != 0,
                        isItalic: (styleFlags & (1 << 1)) != 0,
                        isStrikethrough: (styleFlags & (1 << 2)) != 0
                    )
                    let style = InlineStyle(
                        foregroundColor: foregroundColor,
                        backgroundColor: backgroundColor,
                        fontAttributes: attributes
                    )
                    spans.append(TokenSpan(column: column, length: length, startIndex: startIndex, inlineStyle: style))
                } else {
                    let styleID = buffer[cursor]
                    cursor += 1
                    spans.append(TokenSpan(column: column, length: length, startIndex: startIndex, styleID: styleID))
                }
            }

            lines.append(LineHighlight(spans: spans))
        }

        return DocumentHighlight(lines: lines)
    }

    static func decodeDocumentHighlight(from data: Data) throws -> DocumentHighlight {
        if data.isEmpty {
            return DocumentHighlight()
        }

        let values = toInt32Array(from: data)
        guard values.count >= 3 else {
            throw SweetLineError.invalidResult("Document highlight payload is too short.")
        }

        let flags = Int(values[0])
        let stride = max(Int(values[1]), 0)
        let lineCount = max(Int(values[2]), 0)
        let hasStartIndex = flagsHasStartIndex(flags)
        let usesInlineStyle = flagsUsesInlineStyle(flags)

        guard isValidSpanStride(stride: stride, hasStartIndex: hasStartIndex, usesInlineStyle: usesInlineStyle) else {
            throw SweetLineError.invalidResult("Document highlight stride is invalid.")
        }

        var cursor = 3
        var lines: [LineHighlight] = []
        lines.reserveCapacity(lineCount)

        for _ in 0..<lineCount {
            guard cursor < values.count else {
                throw SweetLineError.invalidResult("Document highlight payload ended before line metadata.")
            }
            let spanCount = max(Int(values[cursor]), 0)
            cursor += 1
            let spans = try decodeSpans(
                values: values,
                cursor: &cursor,
                spanCount: spanCount,
                stride: stride,
                hasStartIndex: hasStartIndex,
                usesInlineStyle: usesInlineStyle
            )
            lines.append(LineHighlight(spans: spans))
        }

        return DocumentHighlight(lines: lines)
    }

    static func decodeLineAnalyzeResult(from data: Data) throws -> LineAnalyzeResult {
        if data.isEmpty {
            return LineAnalyzeResult()
        }

        let values = toInt32Array(from: data)
        guard values.count >= 5 else {
            throw SweetLineError.invalidResult("Line analyze payload is too short.")
        }

        let flags = Int(values[0])
        let stride = max(Int(values[1]), 0)
        let spanCount = max(Int(values[2]), 0)
        let endState = Int(values[3])
        let charCount = Int(values[4])
        let hasStartIndex = flagsHasStartIndex(flags)
        let usesInlineStyle = flagsUsesInlineStyle(flags)

        guard isValidSpanStride(stride: stride, hasStartIndex: hasStartIndex, usesInlineStyle: usesInlineStyle) else {
            throw SweetLineError.invalidResult("Line analyze stride is invalid.")
        }

        var cursor = 5
        let spans = try decodeSpans(
            values: values,
            cursor: &cursor,
            spanCount: spanCount,
            stride: stride,
            hasStartIndex: hasStartIndex,
            usesInlineStyle: usesInlineStyle
        )
        return LineAnalyzeResult(highlight: LineHighlight(spans: spans), endState: endState, charCount: charCount)
    }

    static func decodeDocumentHighlightSlice(from data: Data) throws -> DocumentHighlightSlice {
        if data.isEmpty {
            return DocumentHighlightSlice()
        }

        let values = toInt32Array(from: data)
        guard values.count >= 5 else {
            throw SweetLineError.invalidResult("Highlight slice payload is too short.")
        }

        let flags = Int(values[0])
        let stride = max(Int(values[1]), 0)
        let startLine = Int(values[2])
        let totalLineCount = Int(values[3])
        let lineCount = max(Int(values[4]), 0)
        let hasStartIndex = flagsHasStartIndex(flags)
        let usesInlineStyle = flagsUsesInlineStyle(flags)

        guard isValidSpanStride(stride: stride, hasStartIndex: hasStartIndex, usesInlineStyle: usesInlineStyle) else {
            throw SweetLineError.invalidResult("Highlight slice stride is invalid.")
        }

        var cursor = 5
        var lines: [LineHighlight] = []
        lines.reserveCapacity(lineCount)

        for _ in 0..<lineCount {
            guard cursor < values.count else {
                throw SweetLineError.invalidResult("Highlight slice payload ended before line metadata.")
            }
            let spanCount = max(Int(values[cursor]), 0)
            cursor += 1
            let spans = try decodeSpans(
                values: values,
                cursor: &cursor,
                spanCount: spanCount,
                stride: stride,
                hasStartIndex: hasStartIndex,
                usesInlineStyle: usesInlineStyle
            )
            lines.append(LineHighlight(spans: spans))
        }

        return DocumentHighlightSlice(startLine: startLine, totalLineCount: totalLineCount, lines: lines)
    }

    private static func decodeSpans(
        values: [Int32],
        cursor: inout Int,
        spanCount: Int,
        stride: Int,
        hasStartIndex: Bool,
        usesInlineStyle: Bool
    ) throws -> [TokenSpan] {
        var spans: [TokenSpan] = []
        spans.reserveCapacity(spanCount)

        for _ in 0..<spanCount {
            guard cursor + stride <= values.count else {
                throw SweetLineError.invalidResult("Span payload ended unexpectedly.")
            }

            let column = Int(values[cursor])
            let length = Int(values[cursor + 1])
            cursor += 2

            let startIndex: Int?
            if hasStartIndex {
                startIndex = Int(values[cursor])
                cursor += 1
            } else {
                startIndex = nil
            }

            if usesInlineStyle {
                let foregroundColor = values[cursor]
                let backgroundColor = values[cursor + 1]
                let flags = Int(values[cursor + 2])
                cursor += 3
                let attributes = FontAttributes(
                    isBold: (flags & 1) != 0,
                    isItalic: (flags & (1 << 1)) != 0,
                    isStrikethrough: (flags & (1 << 2)) != 0
                )
                let style = InlineStyle(
                    foregroundColor: foregroundColor,
                    backgroundColor: backgroundColor,
                    fontAttributes: attributes
                )
                spans.append(TokenSpan(column: column, length: length, startIndex: startIndex, inlineStyle: style))
            } else {
                let styleID = values[cursor]
                cursor += 1
                spans.append(TokenSpan(column: column, length: length, startIndex: startIndex, styleID: styleID))
            }
        }

        return spans
    }

    private static func toInt32Array(from data: Data) -> [Int32] {
        let count = data.count / MemoryLayout<Int32>.size
        return data.withUnsafeBytes { rawBuffer in
            let buffer = rawBuffer.bindMemory(to: Int32.self)
            return Array(buffer.prefix(count))
        }
    }

    private static func isValidSpanStride(stride: Int, hasStartIndex: Bool, usesInlineStyle: Bool) -> Bool {
        let expected = 2 + (hasStartIndex ? 1 : 0) + (usesInlineStyle ? 3 : 1)
        return stride == expected
    }

    private static func flagsUsesInlineStyle(_ flags: Int) -> Bool {
        (flags & (1 << 1)) != 0
    }

    private static func flagsHasStartIndex(_ flags: Int) -> Bool {
        (flags & 1) != 0
    }
}
