part of '../sweetline.dart';

class Document {
  Document(this.uri, String text)
    : _handle = using((arena) {
        final uriPtr = _toNativeChar(uri, arena);
        final textPtr = _toNativeChar(text, arena);
        return bindings.sl_create_document(uriPtr, textPtr);
      }) {
    if (_handle == ffi.nullptr) {
      throw SweetLineException('Failed to create document');
    }
  }

  final String uri;
  final bindings.sl_document_handle_t _handle;
  bool _closed = false;

  void close() {
    if (_closed) {
      return;
    }
    _closed = true;
    _throwIfNativeError(bindings.sl_free_document(_handle), 'free document');
  }

  void dispose() => close();

  void _ensureOpen() {
    if (_closed) {
      throw StateError('Document is already closed');
    }
  }
}
