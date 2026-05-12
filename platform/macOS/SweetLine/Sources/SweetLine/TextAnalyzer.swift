import SweetLineCore

public final class TextAnalyzer {
    private let handle: sl_analyzer_handle_t
    private let engine: HighlightEngine
    private var closed = false

    internal init(handle: sl_analyzer_handle_t, engine: HighlightEngine) {
        self.handle = handle
        self.engine = engine
    }

    public func analyzeText(_ text: String) throws -> DocumentHighlight {
        try ensureOpen()
        let result = text.withCString { textPtr in
            sl_text_analyze(handle, textPtr)
        }
        guard let result else {
            return DocumentHighlight()
        }
        defer { sl_free_buffer(result) }
        return NativeBufferParser.readDocumentHighlight(result)
    }

    public func analyzeLine(_ text: String, info: TextLineInfo) throws -> LineAnalyzeResult {
        try ensureOpen()
        var lineInfo = [Int32(info.line), info.startState, Int32(info.startCharOffset)]
        return try lineInfo.withUnsafeMutableBufferPointer { lineInfoBuffer in
            guard let lineInfoPtr = lineInfoBuffer.baseAddress else {
                throw SweetLineError.invalidNativeBuffer("Unable to allocate line info buffer.")
            }
            let result = text.withCString { textPtr in
                sl_text_analyze_line(handle, textPtr, lineInfoPtr)
            }
            guard let result else {
                return LineAnalyzeResult()
            }
            defer { sl_free_buffer(result) }
            return NativeBufferParser.readLineAnalyzeResult(result, lineNumber: info.line)
        }
    }

    public func analyzeIndentGuides(_ text: String) throws -> IndentGuideResult {
        try ensureOpen()
        let result = text.withCString { textPtr in
            sl_text_analyze_indent_guides(handle, textPtr)
        }
        guard let result else {
            return IndentGuideResult()
        }
        defer { sl_free_buffer(result) }
        return NativeBufferParser.readIndentGuideResult(result)
    }

    public func close() {
        closed = true
    }

    private func ensureOpen() throws {
        if closed {
            throw SweetLineError.closedObject("TextAnalyzer")
        }
        _ = try engine.nativeHandle()
    }
}
