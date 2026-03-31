import Foundation
import SweetLineBridge

public final class Document {
    let nativeHandle: sl_document_handle_t

    public let uri: String
    public let text: String

    public init(uri: String, text: String) throws {
        self.uri = uri
        self.text = text
        self.nativeHandle = try NativeInterop.withCString(uri) { cURI in
            try NativeInterop.withCString(text) { cText in
                guard let handle = sl_create_document(cURI, cText) else {
                    throw SweetLineError.nativeError(code: Int(SL_HANDLE_INVALID.rawValue), message: "Failed to create document.")
                }
                return handle
            }
        }
    }

    deinit {
        _ = sl_free_document(nativeHandle)
    }
}
