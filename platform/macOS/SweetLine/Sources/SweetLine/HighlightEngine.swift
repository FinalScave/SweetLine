import SweetLineCore

public final class HighlightEngine {
    private var handle: sl_engine_handle_t?

    public init(config: HighlightConfig = HighlightConfig()) throws {
        guard let handle = sl_create_engine(config.showIndex, config.inlineStyle) else {
            throw SweetLineError.nullHandle(action: "create engine")
        }
        self.handle = handle
    }

    deinit {
        close()
    }

    public func registerStyleName(_ styleName: String, id styleId: Int32) throws {
        let handle = try nativeHandle()
        try styleName.withCString { styleNamePtr in
            try SweetLineNative.throwIfError(
                sl_engine_register_style_name(handle, styleNamePtr, styleId),
                action: "register style name"
            )
        }
    }

    public func getStyleName(id styleId: Int32) throws -> String? {
        let handle = try nativeHandle()
        guard let result = sl_engine_get_style_name(handle, styleId) else {
            return nil
        }
        return String(cString: result)
    }

    public func defineMacro(_ macroName: String) throws {
        let handle = try nativeHandle()
        try macroName.withCString { macroNamePtr in
            try SweetLineNative.throwIfError(
                sl_engine_define_macro(handle, macroNamePtr),
                action: "define macro"
            )
        }
    }

    public func undefineMacro(_ macroName: String) throws {
        let handle = try nativeHandle()
        try macroName.withCString { macroNamePtr in
            try SweetLineNative.throwIfError(
                sl_engine_undefine_macro(handle, macroNamePtr),
                action: "undefine macro"
            )
        }
    }

    public func compileSyntax(fromJson syntaxJson: String) throws {
        let handle = try nativeHandle()
        let error = syntaxJson.withCString { syntaxJsonPtr in
            sl_engine_compile_json(handle, syntaxJsonPtr)
        }
        try SweetLineNative.throwIfSyntaxError(error, action: "compile syntax from JSON")
    }

    public func compileSyntax(fromFile path: String) throws {
        let handle = try nativeHandle()
        let error = path.withCString { pathPtr in
            sl_engine_compile_file(handle, pathPtr)
        }
        try SweetLineNative.throwIfSyntaxError(error, action: "compile syntax from file")
    }

    public func createAnalyzer(syntaxName: String) throws -> TextAnalyzer? {
        let handle = try nativeHandle()
        let analyzerHandle = syntaxName.withCString { syntaxNamePtr in
            sl_engine_create_text_analyzer(handle, syntaxNamePtr)
        }
        guard let analyzerHandle else {
            return nil
        }
        return TextAnalyzer(handle: analyzerHandle, engine: self)
    }

    public func createAnalyzer(fileName: String) throws -> TextAnalyzer? {
        let handle = try nativeHandle()
        let analyzerHandle = fileName.withCString { fileNamePtr in
            sl_engine_create_text_analyzer_by_file_name(handle, fileNamePtr)
        }
        guard let analyzerHandle else {
            return nil
        }
        return TextAnalyzer(handle: analyzerHandle, engine: self)
    }

    public func loadDocument(_ document: Document) throws -> DocumentAnalyzer? {
        let handle = try nativeHandle()
        let documentHandle = try document.nativeHandle()
        guard let analyzerHandle = sl_engine_load_document(handle, documentHandle) else {
            return nil
        }
        return DocumentAnalyzer(handle: analyzerHandle, engine: self, document: document)
    }

    public func close() {
        guard let handle else {
            return
        }
        _ = sl_free_engine(handle)
        self.handle = nil
    }

    internal func nativeHandle() throws -> sl_engine_handle_t {
        guard let handle else {
            throw SweetLineError.closedObject("HighlightEngine")
        }
        return handle
    }
}
