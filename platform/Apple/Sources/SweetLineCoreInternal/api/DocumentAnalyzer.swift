import Foundation
import SweetLineBridge

public final class DocumentAnalyzer {
    private let nativeHandle: sl_analyzer_handle_t

    init(nativeHandle: sl_analyzer_handle_t) {
        self.nativeHandle = nativeHandle
    }

    public func analyze() throws -> DocumentHighlight {
        let result = sl_document_analyze(nativeHandle)
        let data = try NativeInterop.copyInt32Prefix(from: result, initialCount: 3, totalCount: NativeInterop.countDocumentHighlightInts)
        return try HighlightResultDecoder.decodeDocumentHighlight(from: data)
    }

    public func analyzeIncremental(range: TextRange, newText: String) throws -> DocumentHighlight {
        let data = try NativeInterop.withCString(newText) { cText in
            try NativeInterop.withInt32Array([range.start.line, range.start.column, range.end.line, range.end.column]) { changeRange in
                let result = sl_document_analyze_incremental(nativeHandle, changeRange, cText)
                return try NativeInterop.copyInt32Prefix(from: result, initialCount: 3, totalCount: NativeInterop.countDocumentHighlightInts)
            }
        }
        return try HighlightResultDecoder.decodeDocumentHighlight(from: data)
    }

    public func analyzeIncrementalInLineRange(range: TextRange, newText: String, visibleRange: LineRange) throws -> DocumentHighlightSlice {
        let data = try NativeInterop.withCString(newText) { cText in
            try NativeInterop.withInt32Array([range.start.line, range.start.column, range.end.line, range.end.column]) { changeRange in
                try NativeInterop.withInt32Array([visibleRange.startLine, visibleRange.lineCount]) { visible in
                    let result = sl_document_analyze_incremental_in_line_range(nativeHandle, changeRange, cText, visible)
                    return try NativeInterop.copyInt32Prefix(from: result, initialCount: 5, totalCount: NativeInterop.countDocumentHighlightSliceInts)
                }
            }
        }
        return try HighlightResultDecoder.decodeDocumentHighlightSlice(from: data)
    }

    public func analyzeIndentGuides() throws -> IndentGuideResult {
        let result = sl_document_analyze_indent_guides(nativeHandle)
        defer {
            if let result {
                sl_free_buffer(result)
            }
        }
        return try IndentGuideDecoder.decode(from: result)
    }
}
