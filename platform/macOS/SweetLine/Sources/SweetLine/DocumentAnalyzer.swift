import SweetLineCore

public final class DocumentAnalyzer {
    private let handle: sl_analyzer_handle_t
    private let engine: HighlightEngine
    private let document: Document
    private var closed = false

    internal init(handle: sl_analyzer_handle_t, engine: HighlightEngine, document: Document) {
        self.handle = handle
        self.engine = engine
        self.document = document
    }

    public func analyze() throws -> DocumentHighlight {
        try ensureOpen()
        guard let result = sl_document_analyze(handle) else {
            return DocumentHighlight()
        }
        defer { sl_free_buffer(result) }
        return NativeBufferParser.readDocumentHighlight(result)
    }

    public func analyzeLineRange(_ visibleRange: LineRange) throws -> DocumentHighlightSlice {
        try ensureOpen()
        var visible = [Int32(visibleRange.startLine), Int32(visibleRange.lineCount)]
        return try visible.withUnsafeMutableBufferPointer { visibleBuffer in
            guard let visiblePtr = visibleBuffer.baseAddress else {
                throw SweetLineError.invalidNativeBuffer("Unable to allocate visible range buffer.")
            }
            guard let result = sl_document_analyze_line_range(handle, visiblePtr) else {
                return DocumentHighlightSlice()
            }
            defer { sl_free_buffer(result) }
            return NativeBufferParser.readDocumentHighlightSlice(result)
        }
    }

    public func analyzeIncremental(range: TextRange, newText: String) throws -> DocumentHighlight {
        try ensureOpen()
        var change = nativeChangeRange(range)
        return try change.withUnsafeMutableBufferPointer { changeBuffer in
            guard let changePtr = changeBuffer.baseAddress else {
                throw SweetLineError.invalidNativeBuffer("Unable to allocate change range buffer.")
            }
            let result = newText.withCString { newTextPtr in
                sl_document_analyze_incremental(handle, changePtr, newTextPtr)
            }
            guard let result else {
                return DocumentHighlight()
            }
            defer { sl_free_buffer(result) }
            return NativeBufferParser.readDocumentHighlight(result)
        }
    }

    public func analyzeIncrementalInLineRange(
        range: TextRange,
        newText: String,
        visibleRange: LineRange
    ) throws -> DocumentHighlightSlice {
        try ensureOpen()
        var change = nativeChangeRange(range)
        var visible = [Int32(visibleRange.startLine), Int32(visibleRange.lineCount)]
        return try change.withUnsafeMutableBufferPointer { changeBuffer in
            guard let changePtr = changeBuffer.baseAddress else {
                throw SweetLineError.invalidNativeBuffer("Unable to allocate change range buffer.")
            }
            return try visible.withUnsafeMutableBufferPointer { visibleBuffer in
                guard let visiblePtr = visibleBuffer.baseAddress else {
                    throw SweetLineError.invalidNativeBuffer("Unable to allocate visible range buffer.")
                }
                let result = newText.withCString { newTextPtr in
                    sl_document_analyze_incremental_in_line_range(handle, changePtr, newTextPtr, visiblePtr)
                }
                guard let result else {
                    return DocumentHighlightSlice()
                }
                defer { sl_free_buffer(result) }
                return NativeBufferParser.readDocumentHighlightSlice(result)
            }
        }
    }

    public func getHighlightSlice(_ visibleRange: LineRange) throws -> DocumentHighlightSlice {
        try ensureOpen()
        var visible = [Int32(visibleRange.startLine), Int32(visibleRange.lineCount)]
        return try visible.withUnsafeMutableBufferPointer { visibleBuffer in
            guard let visiblePtr = visibleBuffer.baseAddress else {
                throw SweetLineError.invalidNativeBuffer("Unable to allocate visible range buffer.")
            }
            guard let result = sl_document_get_highlight_slice(handle, visiblePtr) else {
                return DocumentHighlightSlice()
            }
            defer { sl_free_buffer(result) }
            return NativeBufferParser.readDocumentHighlightSlice(result)
        }
    }

    public func analyzeIndentGuides() throws -> IndentGuideResult {
        try ensureOpen()
        guard let result = sl_document_analyze_indent_guides(handle) else {
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
            throw SweetLineError.closedObject("DocumentAnalyzer")
        }
        _ = try engine.nativeHandle()
        _ = try document.nativeHandle()
    }

    private func nativeChangeRange(_ range: TextRange) -> [Int32] {
        [
            Int32(range.start.line),
            Int32(range.start.column),
            Int32(range.end.line),
            Int32(range.end.column),
        ]
    }
}
