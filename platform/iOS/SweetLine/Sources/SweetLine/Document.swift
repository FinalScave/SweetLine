import SweetLineCore

public final class Document {
    private var handle: sl_document_handle_t?

    public init(uri: String, text: String) throws {
        let handle = uri.withCString { uriPtr in
            text.withCString { textPtr in
                sl_create_document(uriPtr, textPtr)
            }
        }
        guard let handle else {
            throw SweetLineError.nullHandle(action: "create document")
        }
        self.handle = handle
    }

    deinit {
        close()
    }

    public func close() {
        guard let handle else {
            return
        }
        _ = sl_free_document(handle)
        self.handle = nil
    }

    internal func nativeHandle() throws -> sl_document_handle_t {
        guard let handle else {
            throw SweetLineError.closedObject("Document")
        }
        return handle
    }
}
