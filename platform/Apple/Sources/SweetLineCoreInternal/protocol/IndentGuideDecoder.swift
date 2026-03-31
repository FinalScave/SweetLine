import Foundation

enum IndentGuideDecoder {
    static func decode(from buffer: UnsafeMutablePointer<Int32>?) throws -> IndentGuideResult {
        guard let buffer else {
            return IndentGuideResult()
        }

        let guideCount = max(Int(buffer[0]), 0)
        let guideStride = Int(buffer[1])
        let lineStateCount = max(Int(buffer[2]), 0)
        let lineStateStride = Int(buffer[3])

        guard guideStride == 6 else {
            throw SweetLineError.invalidResult("Indent guide stride is invalid.")
        }
        guard lineStateStride == 4 else {
            throw SweetLineError.invalidResult("Indent guide line-state stride is invalid.")
        }

        var cursor = 4
        var guides: [IndentGuide] = []
        guides.reserveCapacity(guideCount)

        for _ in 0..<guideCount {
            let column = Int(buffer[cursor])
            let startLine = Int(buffer[cursor + 1])
            let endLine = Int(buffer[cursor + 2])
            let nestingLevel = Int(buffer[cursor + 3])
            let branchCount = max(Int(buffer[cursor + 5]), 0)

            guides.append(
                IndentGuide(
                    column: column,
                    startLine: startLine,
                    endLine: endLine,
                    nestingLevel: nestingLevel
                )
            )

            cursor += guideStride + branchCount * 2
        }

        var lineStates: [IndentGuideLineState] = []
        lineStates.reserveCapacity(lineStateCount)

        for _ in 0..<lineStateCount {
            lineStates.append(
                IndentGuideLineState(
                    nestingLevel: Int(buffer[cursor]),
                    scopeState: Int(buffer[cursor + 1]),
                    scopeColumn: Int(buffer[cursor + 2]),
                    indentLevel: Int(buffer[cursor + 3])
                )
            )
            cursor += lineStateStride
        }

        return IndentGuideResult(guides: guides, lineStates: lineStates)
    }

    static func decode(from data: Data) throws -> IndentGuideResult {
        if data.isEmpty {
            return IndentGuideResult()
        }

        let values = toInt32Array(from: data)
        guard values.count >= 4 else {
            throw SweetLineError.invalidResult("Indent guide payload is too short.")
        }

        let guideCount = max(Int(values[0]), 0)
        let lineStateCount = max(Int(values[2]), 0)

        var cursor = 4
        var guides: [IndentGuide] = []
        guides.reserveCapacity(guideCount)

        for _ in 0..<guideCount {
            guard cursor + 6 <= values.count else {
                throw SweetLineError.invalidResult("Indent guide payload ended before guide header.")
            }

            let column = Int(values[cursor])
            let startLine = Int(values[cursor + 1])
            let endLine = Int(values[cursor + 2])
            let nestingLevel = Int(values[cursor + 3])
            let branchCount = max(Int(values[cursor + 5]), 0)

            cursor += 6 + branchCount * 2
            guard cursor <= values.count else {
                throw SweetLineError.invalidResult("Indent guide payload ended before branch data completed.")
            }

            guides.append(IndentGuide(column: column, startLine: startLine, endLine: endLine, nestingLevel: nestingLevel))
        }

        var lineStates: [IndentGuideLineState] = []
        lineStates.reserveCapacity(lineStateCount)

        for _ in 0..<lineStateCount {
            guard cursor + 4 <= values.count else {
                throw SweetLineError.invalidResult("Indent guide payload ended before line state data.")
            }
            lineStates.append(
                IndentGuideLineState(
                    nestingLevel: Int(values[cursor]),
                    scopeState: Int(values[cursor + 1]),
                    scopeColumn: Int(values[cursor + 2]),
                    indentLevel: Int(values[cursor + 3])
                )
            )
            cursor += 4
        }

        return IndentGuideResult(guides: guides, lineStates: lineStates)
    }

    private static func toInt32Array(from data: Data) -> [Int32] {
        let count = data.count / MemoryLayout<Int32>.size
        return data.withUnsafeBytes { rawBuffer in
            let buffer = rawBuffer.bindMemory(to: Int32.self)
            return Array(buffer.prefix(count))
        }
    }
}
