import SweetLineCore

enum SweetLineNative {
    static func codeValue(_ code: sl_error_t) -> Int32 {
        Int32(code.rawValue)
    }

    static func throwIfError(_ code: sl_error_t, action: String) throws {
        guard code == SL_OK else {
            throw SweetLineError.nativeError(code: codeValue(code), action: action)
        }
    }

    static func throwIfSyntaxError(_ error: sl_syntax_error_t, action: String) throws {
        guard error.err_code == SL_OK else {
            let message: String
            if let errorMessage = error.err_msg {
                message = String(cString: errorMessage)
            } else {
                message = "\(action) failed"
            }
            throw SyntaxCompileError(code: codeValue(error.err_code), message: message)
        }
    }
}
