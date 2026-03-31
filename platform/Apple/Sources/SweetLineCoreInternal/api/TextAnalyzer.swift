import Foundation
import SweetLineBridge

public final class TextAnalyzer {
    fileprivate let nativeHandle: sl_analyzer_handle_t

    init(nativeHandle: sl_analyzer_handle_t) {
        self.nativeHandle = nativeHandle
    }

    public func analyze(_ text: String) throws -> DocumentHighlight {
        try NativeInterop.withCString(text) { cText in
            let result = sl_text_analyze(nativeHandle, cText)
            defer {
                if let result {
                    sl_free_buffer(result)
                }
            }
            return try HighlightResultDecoder.decodeDocumentHighlight(from: result)
        }
    }

    public func analyzeLine(_ text: String, info: TextLineInfo) throws -> LineAnalyzeResult {
        let data = try NativeInterop.withCString(text) { cText in
            try NativeInterop.withInt32Array([info.line, info.startState, info.startCharOffset]) { lineInfo in
                let result = sl_text_analyze_line(nativeHandle, cText, lineInfo)
                return try NativeInterop.copyInt32Prefix(from: result, initialCount: 5, totalCount: NativeInterop.countLineAnalyzeInts)
            }
        }
        return try HighlightResultDecoder.decodeLineAnalyzeResult(from: data)
    }

    public func analyzeIndentGuides(_ text: String) throws -> IndentGuideResult {
        try NativeInterop.withCString(text) { cText in
            let result = sl_text_analyze_indent_guides(nativeHandle, cText)
            defer {
                if let result {
                    sl_free_buffer(result)
                }
            }
            return try IndentGuideDecoder.decode(from: result)
        }
    }
}
