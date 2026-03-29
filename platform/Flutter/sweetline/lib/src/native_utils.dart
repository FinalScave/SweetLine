part of '../sweetline.dart';

ffi.Pointer<ffi.Char> _toNativeChar(String value, ffi.Allocator allocator) {
  return value.toNativeUtf8(allocator: allocator).cast<ffi.Char>();
}

String _readNativeString(ffi.Pointer<ffi.Char> ptr) {
  if (ptr == ffi.nullptr) {
    return '';
  }
  return ptr.cast<Utf8>().toDartString();
}

int _readInt(ffi.Pointer<ffi.Int32> bufferPtr, int index) {
  return (bufferPtr + index).value;
}

int _clampNonNegative(int value) => value < 0 ? 0 : value;

bool _flagsHasStartIndex(int flags) => (flags & 1) != 0;

bool _flagsUsesInlineStyle(int flags) => (flags & (1 << 1)) != 0;

bool _isValidSpanStride(int stride, bool hasStartIndex, bool inlineStyle) {
  final expected = 2 + (hasStartIndex ? 1 : 0) + (inlineStyle ? 3 : 1);
  return stride == expected;
}

SweetLineErrorCode _mapErrorCode(bindings.sl_error code) {
  switch (code) {
    case bindings.sl_error.SL_OK:
      return SweetLineErrorCode.ok;
    case bindings.sl_error.SL_HANDLE_INVALID:
      return SweetLineErrorCode.handleInvalid;
    case bindings.sl_error.SL_JSON_PROPERTY_MISSED:
      return SweetLineErrorCode.jsonPropertyMissed;
    case bindings.sl_error.SL_JSON_PROPERTY_INVALID:
      return SweetLineErrorCode.jsonPropertyInvalid;
    case bindings.sl_error.SL_PATTERN_INVALID:
      return SweetLineErrorCode.patternInvalid;
    case bindings.sl_error.SL_STATE_INVALID:
      return SweetLineErrorCode.stateInvalid;
    case bindings.sl_error.SL_JSON_INVALID:
      return SweetLineErrorCode.jsonInvalid;
    case bindings.sl_error.SL_FILE_IO_ERR:
      return SweetLineErrorCode.fileIoError;
    case bindings.sl_error.SL_FILE_EMPTY:
      return SweetLineErrorCode.fileEmpty;
  }
}

void _throwIfNativeError(bindings.sl_error code, String action) {
  if (code == bindings.sl_error.SL_OK) {
    return;
  }
  throw SweetLineException('$action failed', code: _mapErrorCode(code));
}

void _throwIfSyntaxError(bindings.sl_syntax_error_t error, String action) {
  if (error.err_code == bindings.sl_error.SL_OK) {
    return;
  }
  final message = _readNativeString(error.err_msg);
  throw SyntaxCompileError(
    message.isEmpty ? '$action failed' : message,
    code: _mapErrorCode(error.err_code),
  );
}

T _parseOwnedBuffer<T>(
  ffi.Pointer<ffi.Int32> bufferPtr,
  T emptyValue,
  T Function(ffi.Pointer<ffi.Int32> bufferPtr) parser,
) {
  if (bufferPtr == ffi.nullptr) {
    return emptyValue;
  }
  try {
    return parser(bufferPtr);
  } finally {
    bindings.sl_free_buffer(bufferPtr);
  }
}
