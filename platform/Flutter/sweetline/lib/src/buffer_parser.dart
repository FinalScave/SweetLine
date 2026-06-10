part of '../sweetline.dart';

final class _BufferParser {
  static DocumentHighlight readDocumentHighlight(
    ffi.Pointer<ffi.Int32> bufferPtr,
  ) {
    final highlight = DocumentHighlight();
    if (bufferPtr == ffi.nullptr) {
      return highlight;
    }

    final flags = _readInt(bufferPtr, 0);
    final stride = _clampNonNegative(_readInt(bufferPtr, 1));
    final lineCount = _clampNonNegative(_readInt(bufferPtr, 2));
    final hasStartIndex = _flagsHasStartIndex(flags);
    final inlineStyle = _flagsUsesInlineStyle(flags);
    if (!_isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
      return highlight;
    }

    var index = 3;
    for (var line = 0; line < lineCount; line++) {
      final lineHighlight = LineHighlight();
      highlight.lines.add(lineHighlight);
      final spanCount = _clampNonNegative(_readInt(bufferPtr, index++));
      for (var span = 0; span < spanCount; span++) {
        final parsed = _readTokenSpan(
          bufferPtr,
          index: index,
          line: line,
          hasStartIndex: hasStartIndex,
          inlineStyle: inlineStyle,
        );
        index = parsed.nextIndex;
        lineHighlight.spans.add(parsed.span);
      }
    }

    return highlight;
  }

  static DocumentHighlightSlice readDocumentHighlightSlice(
    ffi.Pointer<ffi.Int32> bufferPtr,
  ) {
    if (bufferPtr == ffi.nullptr) {
      return DocumentHighlightSlice();
    }

    final flags = _readInt(bufferPtr, 0);
    final stride = _clampNonNegative(_readInt(bufferPtr, 1));
    final startLine = _readInt(bufferPtr, 2);
    final totalLineCount = _readInt(bufferPtr, 3);
    final lineCount = _clampNonNegative(_readInt(bufferPtr, 4));
    final hasStartIndex = _flagsHasStartIndex(flags);
    final inlineStyle = _flagsUsesInlineStyle(flags);
    if (!_isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
      return DocumentHighlightSlice(
        startLine: startLine,
        totalLineCount: totalLineCount,
      );
    }

    final lines = <LineHighlight>[
      for (var i = 0; i < lineCount; i++) LineHighlight(),
    ];

    var index = 5;
    for (var i = 0; i < lineCount; i++) {
      final line = startLine + i;
      final spanCount = _clampNonNegative(_readInt(bufferPtr, index++));
      final lineHighlight = lines[i];
      for (var span = 0; span < spanCount; span++) {
        final parsed = _readTokenSpan(
          bufferPtr,
          index: index,
          line: line,
          hasStartIndex: hasStartIndex,
          inlineStyle: inlineStyle,
        );
        index = parsed.nextIndex;
        lineHighlight.spans.add(parsed.span);
      }
    }

    return DocumentHighlightSlice(
      startLine: startLine,
      totalLineCount: totalLineCount,
      lines: lines,
    );
  }

  static LineAnalyzeResult readLineAnalyzeResult(
    ffi.Pointer<ffi.Int32> bufferPtr, [
    int lineNumber = 0,
  ]) {
    if (bufferPtr == ffi.nullptr) {
      return LineAnalyzeResult(LineHighlight(), 0, 0);
    }

    final flags = _readInt(bufferPtr, 0);
    final stride = _clampNonNegative(_readInt(bufferPtr, 1));
    final spanCount = _clampNonNegative(_readInt(bufferPtr, 2));
    final endState = _readInt(bufferPtr, 3);
    final charCount = _readInt(bufferPtr, 4);
    final hasStartIndex = _flagsHasStartIndex(flags);
    final inlineStyle = _flagsUsesInlineStyle(flags);
    if (!_isValidSpanStride(stride, hasStartIndex, inlineStyle)) {
      return LineAnalyzeResult(LineHighlight(), endState, charCount);
    }

    final lineHighlight = LineHighlight();
    var index = 5;
    for (var i = 0; i < spanCount; i++) {
      final parsed = _readTokenSpan(
        bufferPtr,
        index: index,
        line: lineNumber,
        hasStartIndex: hasStartIndex,
        inlineStyle: inlineStyle,
      );
      index = parsed.nextIndex;
      lineHighlight.spans.add(parsed.span);
    }

    return LineAnalyzeResult(lineHighlight, endState, charCount);
  }

  static IndentGuideResult readIndentGuideResult(
    ffi.Pointer<ffi.Int32> bufferPtr,
  ) {
    if (bufferPtr == ffi.nullptr) {
      return IndentGuideResult();
    }

    final startLine = _readInt(bufferPtr, 0);
    final lineStateCount = _clampNonNegative(_readInt(bufferPtr, 1));
    final guideCount = _clampNonNegative(_readInt(bufferPtr, 2));
    final result = IndentGuideResult(startLine: startLine);

    var index = 3;
    for (var i = 0; i < guideCount; i++) {
      final column = _readInt(bufferPtr, index++);
      final startLine = _readInt(bufferPtr, index++);
      final endLine = _readInt(bufferPtr, index++);
      final flags = _readInt(bufferPtr, index++);
      final continuesBefore = (flags & 1) != 0;
      final continuesAfter = (flags & (1 << 1)) != 0;
      final branchCount = _clampNonNegative(_readInt(bufferPtr, index++));

      final branches = <IndentGuideBranchPoint>[];
      for (var branch = 0; branch < branchCount; branch++) {
        final branchLine = _readInt(bufferPtr, index++);
        final branchColumn = _readInt(bufferPtr, index++);
        branches.add(IndentGuideBranchPoint(branchLine, branchColumn));
      }

      result.guideLines.add(
        IndentGuideLine(
          column,
          startLine,
          endLine,
          continuesBefore,
          continuesAfter,
          branches,
        ),
      );
    }

    for (var i = 0; i < lineStateCount; i++) {
      result.lineStates.add(
        LineScopeState(
          _readInt(bufferPtr, index++),
          _readInt(bufferPtr, index++),
          _readInt(bufferPtr, index++),
          _readInt(bufferPtr, index++),
        ),
      );
    }

    return result;
  }

  static BracketPairResult readBracketPairResult(
    ffi.Pointer<ffi.Int32> bufferPtr,
  ) {
    if (bufferPtr == ffi.nullptr) {
      return BracketPairResult();
    }

    final flags = _readInt(bufferPtr, 0);
    final stride = _clampNonNegative(_readInt(bufferPtr, 1));
    final lineCount = _clampNonNegative(_readInt(bufferPtr, 2));
    final hasStartIndex = _flagsHasStartIndex(flags);
    if (!_isValidBracketTokenStride(stride, hasStartIndex)) {
      return BracketPairResult();
    }

    final parsed = _readBracketLines(
      bufferPtr,
      index: 3,
      startLine: 0,
      lineCount: lineCount,
      hasStartIndex: hasStartIndex,
    );
    return BracketPairResult(totalLineCount: lineCount, lines: parsed.lines);
  }

  static BracketPairResult readBracketPairResultSlice(
    ffi.Pointer<ffi.Int32> bufferPtr,
  ) {
    if (bufferPtr == ffi.nullptr) {
      return BracketPairResult();
    }

    final flags = _readInt(bufferPtr, 0);
    final stride = _clampNonNegative(_readInt(bufferPtr, 1));
    final startLine = _readInt(bufferPtr, 2);
    final totalLineCount = _readInt(bufferPtr, 3);
    final lineCount = _clampNonNegative(_readInt(bufferPtr, 4));
    final hasStartIndex = _flagsHasStartIndex(flags);
    if (!_isValidBracketTokenStride(stride, hasStartIndex)) {
      return BracketPairResult(
        startLine: startLine,
        totalLineCount: totalLineCount,
      );
    }

    final parsed = _readBracketLines(
      bufferPtr,
      index: 5,
      startLine: startLine,
      lineCount: lineCount,
      hasStartIndex: hasStartIndex,
    );
    return BracketPairResult(
      startLine: startLine,
      totalLineCount: totalLineCount,
      lines: parsed.lines,
    );
  }

  static ({TokenSpan span, int nextIndex}) _readTokenSpan(
    ffi.Pointer<ffi.Int32> bufferPtr, {
    required int index,
    required int line,
    required bool hasStartIndex,
    required bool inlineStyle,
  }) {
    final startColumn = _readInt(bufferPtr, index++);
    final length = _readInt(bufferPtr, index++);
    final startIndex = hasStartIndex ? _readInt(bufferPtr, index++) : 0;
    final endColumn = startColumn + length;
    final endIndex = hasStartIndex ? startIndex + length : 0;
    final range = TextRange(
      TextPosition(line, startColumn, startIndex),
      TextPosition(line, endColumn, endIndex),
    );

    if (inlineStyle) {
      final style = InlineStyle(
        _readInt(bufferPtr, index++),
        _readInt(bufferPtr, index++),
        _readInt(bufferPtr, index++),
      );
      return (span: TokenSpan.inlineStyle(range, style), nextIndex: index);
    }

    return (
      span: TokenSpan.styleId(range, _readInt(bufferPtr, index++)),
      nextIndex: index,
    );
  }

  static ({List<LineBracketPairs> lines, int nextIndex}) _readBracketLines(
    ffi.Pointer<ffi.Int32> bufferPtr, {
    required int index,
    required int startLine,
    required int lineCount,
    required bool hasStartIndex,
  }) {
    final lines = <LineBracketPairs>[];
    for (var i = 0; i < lineCount; i++) {
      final line = startLine + i;
      final tokenCount = _clampNonNegative(_readInt(bufferPtr, index++));
      final lineResult = LineBracketPairs();
      lines.add(lineResult);
      for (var token = 0; token < tokenCount; token++) {
        final column = _readInt(bufferPtr, index++);
        final length = _readInt(bufferPtr, index++);
        final tokenStartIndex = hasStartIndex
            ? _readInt(bufferPtr, index++)
            : 0;
        final depth = _readInt(bufferPtr, index++);
        final kind = _readInt(bufferPtr, index++);
        final matchState = _readInt(bufferPtr, index++);
        final partnerLine = _readInt(bufferPtr, index++);
        final partnerColumn = _readInt(bufferPtr, index++);
        final partnerLength = _readInt(bufferPtr, index++);
        final partnerStartIndex = hasStartIndex
            ? _readInt(bufferPtr, index++)
            : 0;
        final range = TextRange(
          TextPosition(line, column, tokenStartIndex),
          TextPosition(
            line,
            column + length,
            hasStartIndex ? tokenStartIndex + length : 0,
          ),
        );
        TextRange? partnerRange;
        if (partnerLine >= 0 && partnerColumn >= 0 && partnerLength >= 0) {
          partnerRange = TextRange(
            TextPosition(partnerLine, partnerColumn, partnerStartIndex),
            TextPosition(
              partnerLine,
              partnerColumn + partnerLength,
              hasStartIndex ? partnerStartIndex + partnerLength : 0,
            ),
          );
        }
        lineResult.tokens.add(
          BracketToken(
            range: range,
            depth: depth,
            kind: kind == 1 ? BracketTokenKind.close : BracketTokenKind.open,
            matchState: switch (matchState) {
              0 => BracketMatchState.matched,
              1 => BracketMatchState.unmatched,
              _ => BracketMatchState.unknown,
            },
            partnerRange: partnerRange,
          ),
        );
      }
    }
    return (lines: lines, nextIndex: index);
  }
}
