part of '../sweetline.dart';

class HighlightEngine {
  HighlightEngine([this.config = const HighlightConfig()])
    : _handle = bindings.sl_create_engine(
        config.showIndex,
        config.inlineStyle,
      ) {
    if (_handle == ffi.nullptr) {
      throw SweetLineException('Failed to create SweetLine engine');
    }
  }

  final HighlightConfig config;
  final bindings.sl_engine_handle_t _handle;
  bool _closed = false;

  void registerStyleName(String styleName, int styleId) {
    _ensureOpen();
    using((arena) {
      final styleNamePtr = _toNativeChar(styleName, arena);
      _throwIfNativeError(
        bindings.sl_engine_register_style_name(_handle, styleNamePtr, styleId),
        'register style name',
      );
    });
  }

  String? getStyleName(int styleId) {
    _ensureOpen();
    final resultPtr = bindings.sl_engine_get_style_name(_handle, styleId);
    if (resultPtr == ffi.nullptr) {
      return null;
    }
    return _readNativeString(resultPtr);
  }

  void defineMacro(String macroName) {
    _ensureOpen();
    using((arena) {
      final macroNamePtr = _toNativeChar(macroName, arena);
      _throwIfNativeError(
        bindings.sl_engine_define_macro(_handle, macroNamePtr),
        'define macro',
      );
    });
  }

  void undefineMacro(String macroName) {
    _ensureOpen();
    using((arena) {
      final macroNamePtr = _toNativeChar(macroName, arena);
      _throwIfNativeError(
        bindings.sl_engine_undefine_macro(_handle, macroNamePtr),
        'undefine macro',
      );
    });
  }

  void compileSyntaxFromJson(String syntaxJson) {
    _ensureOpen();
    using((arena) {
      final syntaxPtr = _toNativeChar(syntaxJson, arena);
      final error = bindings.sl_engine_compile_json(_handle, syntaxPtr);
      _throwIfSyntaxError(error, 'compile syntax from JSON');
    });
  }

  void compileSyntaxFromFile(String path) {
    _ensureOpen();
    using((arena) {
      final pathPtr = _toNativeChar(path, arena);
      final error = bindings.sl_engine_compile_file(_handle, pathPtr);
      _throwIfSyntaxError(error, 'compile syntax from file');
    });
  }

  TextAnalyzer? createAnalyzerBySyntaxName(String syntaxName) {
    _ensureOpen();
    return using((arena) {
      final syntaxNamePtr = _toNativeChar(syntaxName, arena);
      final handle = bindings.sl_engine_create_text_analyzer(
        _handle,
        syntaxNamePtr,
      );
      if (handle == ffi.nullptr) {
        return null;
      }
      return TextAnalyzer._(this, handle);
    });
  }

  TextAnalyzer? createAnalyzerByFileName(String fileName) {
    _ensureOpen();
    return using((arena) {
      final fileNamePtr = _toNativeChar(fileName, arena);
      final handle = bindings.sl_engine_create_text_analyzer_by_file_name(
        _handle,
        fileNamePtr,
      );
      if (handle == ffi.nullptr) {
        return null;
      }
      return TextAnalyzer._(this, handle);
    });
  }

  DocumentAnalyzer? loadDocument(Document document) {
    _ensureOpen();
    document._ensureOpen();
    final handle = bindings.sl_engine_load_document(_handle, document._handle);
    if (handle == ffi.nullptr) {
      return null;
    }
    return DocumentAnalyzer._(this, document, handle);
  }

  void removeDocument(String uri) {
    _ensureOpen();
    using((arena) {
      final uriPtr = _toNativeChar(uri, arena);
      _throwIfNativeError(
        bindings.sl_engine_remove_document(_handle, uriPtr),
        'remove document',
      );
    });
  }

  void close() {
    if (_closed) {
      return;
    }
    _closed = true;
    _throwIfNativeError(bindings.sl_free_engine(_handle), 'free engine');
  }

  void dispose() => close();

  void _ensureOpen() {
    if (_closed) {
      throw StateError('HighlightEngine is already closed');
    }
  }
}
