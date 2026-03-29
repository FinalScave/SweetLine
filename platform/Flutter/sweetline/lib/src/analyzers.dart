part of '../sweetline.dart';

class TextAnalyzer {
  TextAnalyzer._(this._engine, this._handle);

  final HighlightEngine _engine;
  final bindings.sl_analyzer_handle_t _handle;
  bool _closed = false;

  DocumentHighlight analyzeText(String text) {
    _ensureOpen();
    return using((arena) {
      final textPtr = _toNativeChar(text, arena);
      final resultPtr = bindings.sl_text_analyze(_handle, textPtr);
      return _parseOwnedBuffer(
        resultPtr,
        DocumentHighlight(),
        _BufferParser.readDocumentHighlight,
      );
    });
  }

  LineAnalyzeResult analyzeLine(String text, TextLineInfo info) {
    _ensureOpen();
    return using((arena) {
      final textPtr = _toNativeChar(text, arena);
      final lineInfoPtr = arena.allocate<ffi.Int32>(
        3 * ffi.sizeOf<ffi.Int32>(),
      );
      final lineInfo = lineInfoPtr.asTypedList(3);
      lineInfo[0] = info.line;
      lineInfo[1] = info.startState;
      lineInfo[2] = info.startCharOffset;

      final resultPtr = bindings.sl_text_analyze_line(
        _handle,
        textPtr,
        lineInfoPtr,
      );
      return _parseOwnedBuffer(
        resultPtr,
        LineAnalyzeResult(LineHighlight(), 0, 0),
        (bufferPtr) =>
            _BufferParser.readLineAnalyzeResult(bufferPtr, info.line),
      );
    });
  }

  IndentGuideResult analyzeIndentGuides(String text) {
    _ensureOpen();
    return using((arena) {
      final textPtr = _toNativeChar(text, arena);
      final resultPtr = bindings.sl_text_analyze_indent_guides(
        _handle,
        textPtr,
      );
      return _parseOwnedBuffer(
        resultPtr,
        IndentGuideResult(),
        _BufferParser.readIndentGuideResult,
      );
    });
  }

  void close() {
    _closed = true;
  }

  void dispose() => close();

  void _ensureOpen() {
    if (_closed) {
      throw StateError('TextAnalyzer is already closed');
    }
    _engine._ensureOpen();
  }
}

class DocumentAnalyzer {
  DocumentAnalyzer._(this._engine, this._document, this._handle);

  final HighlightEngine _engine;
  final Document _document;
  final bindings.sl_analyzer_handle_t _handle;
  bool _closed = false;

  DocumentHighlight analyze() {
    _ensureOpen();
    final resultPtr = bindings.sl_document_analyze(_handle);
    return _parseOwnedBuffer(
      resultPtr,
      DocumentHighlight(),
      _BufferParser.readDocumentHighlight,
    );
  }

  DocumentHighlight analyzeIncremental(TextRange range, String newText) {
    _ensureOpen();
    return using((arena) {
      final rangePtr = arena.allocate<ffi.Int32>(4 * ffi.sizeOf<ffi.Int32>());
      final rangeValues = rangePtr.asTypedList(4);
      rangeValues[0] = range.start.line;
      rangeValues[1] = range.start.column;
      rangeValues[2] = range.end.line;
      rangeValues[3] = range.end.column;

      final textPtr = _toNativeChar(newText, arena);
      final resultPtr = bindings.sl_document_analyze_incremental(
        _handle,
        rangePtr,
        textPtr,
      );
      return _parseOwnedBuffer(
        resultPtr,
        DocumentHighlight(),
        _BufferParser.readDocumentHighlight,
      );
    });
  }

  DocumentHighlightSlice analyzeIncrementalInLineRange(
    TextRange range,
    String newText,
    LineRange visibleRange,
  ) {
    _ensureOpen();
    return using((arena) {
      final rangePtr = arena.allocate<ffi.Int32>(4 * ffi.sizeOf<ffi.Int32>());
      final rangeValues = rangePtr.asTypedList(4);
      rangeValues[0] = range.start.line;
      rangeValues[1] = range.start.column;
      rangeValues[2] = range.end.line;
      rangeValues[3] = range.end.column;

      final visiblePtr = arena.allocate<ffi.Int32>(2 * ffi.sizeOf<ffi.Int32>());
      final visibleValues = visiblePtr.asTypedList(2);
      visibleValues[0] = visibleRange.startLine;
      visibleValues[1] = visibleRange.lineCount;

      final textPtr = _toNativeChar(newText, arena);
      final resultPtr = bindings.sl_document_analyze_incremental_in_line_range(
        _handle,
        rangePtr,
        textPtr,
        visiblePtr,
      );
      return _parseOwnedBuffer(
        resultPtr,
        DocumentHighlightSlice(),
        _BufferParser.readDocumentHighlightSlice,
      );
    });
  }

  DocumentHighlightSlice getHighlightSlice(LineRange visibleRange) {
    _ensureOpen();
    return using((arena) {
      final visiblePtr = arena.allocate<ffi.Int32>(2 * ffi.sizeOf<ffi.Int32>());
      final visibleValues = visiblePtr.asTypedList(2);
      visibleValues[0] = visibleRange.startLine;
      visibleValues[1] = visibleRange.lineCount;

      final resultPtr = bindings.sl_document_get_highlight_slice(
        _handle,
        visiblePtr,
      );
      return _parseOwnedBuffer(
        resultPtr,
        DocumentHighlightSlice(),
        _BufferParser.readDocumentHighlightSlice,
      );
    });
  }

  IndentGuideResult analyzeIndentGuides() {
    _ensureOpen();
    final resultPtr = bindings.sl_document_analyze_indent_guides(_handle);
    return _parseOwnedBuffer(
      resultPtr,
      IndentGuideResult(),
      _BufferParser.readIndentGuideResult,
    );
  }

  void close() {
    _closed = true;
  }

  void dispose() => close();

  void _ensureOpen() {
    if (_closed) {
      throw StateError('DocumentAnalyzer is already closed');
    }
    _engine._ensureOpen();
    _document._ensureOpen();
  }
}
