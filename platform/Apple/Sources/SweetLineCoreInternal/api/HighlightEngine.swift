import Foundation
import SweetLineBridge

public final class HighlightEngine {
    private let nativeHandle: sl_engine_handle_t
    public let config: HighlightConfig

    public init(config: HighlightConfig = .default) throws {
        self.config = config
        guard let nativeHandle = sl_create_engine(config.showIndex, config.inlineStyle) else {
            throw SweetLineError.nativeError(code: Int(SL_HANDLE_INVALID.rawValue), message: "Failed to create engine.")
        }
        self.nativeHandle = nativeHandle
    }

    deinit {
        _ = sl_free_engine(nativeHandle)
    }

    public func compileSyntax(fromFile path: String) throws {
        let syntaxError = NativeInterop.withCString(path) { cPath in
            sl_engine_compile_file(nativeHandle, cPath)
        }
        try NativeInterop.syntaxError(syntaxError)
    }

    public func compileSyntax(fromJSON json: String) throws {
        let syntaxError = NativeInterop.withCString(json) { cJSON in
            sl_engine_compile_json(nativeHandle, cJSON)
        }
        try NativeInterop.syntaxError(syntaxError)
    }

    public func registerStyleName(_ styleName: String, styleID: Int32) throws {
        let error = NativeInterop.withCString(styleName) { cStyleName in
            sl_engine_register_style_name(nativeHandle, cStyleName, styleID)
        }
        try NativeInterop.nativeError(error, action: "register style name")
    }

    public func defineMacro(_ name: String) throws {
        let error = NativeInterop.withCString(name) { cName in
            sl_engine_define_macro(nativeHandle, cName)
        }
        try NativeInterop.nativeError(error, action: "define macro")
    }

    public func undefineMacro(_ name: String) throws {
        let error = NativeInterop.withCString(name) { cName in
            sl_engine_undefine_macro(nativeHandle, cName)
        }
        try NativeInterop.nativeError(error, action: "undefine macro")
    }

    public func createAnalyzer(named syntaxName: String) throws -> TextAnalyzer {
        let analyzer = NativeInterop.withCString(syntaxName) { cSyntaxName in
            sl_engine_create_text_analyzer(nativeHandle, cSyntaxName)
        }
        guard let analyzer else {
            throw SweetLineError.unsupportedSyntax(syntaxName)
        }
        return TextAnalyzer(nativeHandle: analyzer)
    }

    public func createAnalyzer(forExtension pathExtension: String) throws -> TextAnalyzer {
        let analyzer = NativeInterop.withCString(pathExtension) { cExtension in
            sl_engine_create_text_analyzer2(nativeHandle, cExtension)
        }
        guard let analyzer else {
            throw SweetLineError.unsupportedSyntax(pathExtension)
        }
        return TextAnalyzer(nativeHandle: analyzer)
    }

    public func loadDocument(_ document: Document) throws -> DocumentAnalyzer {
        guard let analyzer = sl_engine_load_document(nativeHandle, document.nativeHandle) else {
            throw SweetLineError.invalidResult("Native engine failed to load document.")
        }
        return DocumentAnalyzer(nativeHandle: analyzer)
    }
}
