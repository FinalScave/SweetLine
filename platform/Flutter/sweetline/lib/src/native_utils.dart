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

bool _isValidBracketTokenStride(int stride, bool hasStartIndex) {
  final expected = 8 + (hasStartIndex ? 2 : 0);
  return stride == expected;
}

int _mapErrorCode(bindings.sl_error code) {
  return code.value;
}

void _throwIfNativeError(bindings.sl_error code, String action) {
  if (code == bindings.sl_error.SL_OK) {
    return;
  }
  throw SweetLineException('$action failed', errorCode: _mapErrorCode(code));
}

void _throwIfSyntaxError(bindings.sl_syntax_error_t error, String action) {
  if (error.err_code == bindings.sl_error.SL_OK) {
    return;
  }
  final message = _readNativeString(error.err_msg);
  throw SyntaxCompileError(
    message.isEmpty ? '$action failed' : message,
    errorCode: _mapErrorCode(error.err_code),
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
