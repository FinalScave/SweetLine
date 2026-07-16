part of '../sweetline.dart';

class HighlightConfig {
  const HighlightConfig({
    this.showIndex = false,
    this.inlineStyle = false,
    this.tabSize = 4,
  });

  final bool showIndex;
  final bool inlineStyle;
  final int tabSize;
}

class TextPosition {
  const TextPosition(this.line, this.column, [this.index = 0]);

  final int line;
  final int column;
  final int index;

  @override
  String toString() =>
      'TextPosition(line: $line, column: $column, index: $index)';
}

class TextRange {
  const TextRange(this.start, this.end);

  final TextPosition start;
  final TextPosition end;

  @override
  String toString() => 'TextRange(start: $start, end: $end)';
}

class TextLineInfo {
  const TextLineInfo(this.line, this.startState, [this.startCharOffset = 0]);

  final int line;
  final int startState;
  final int startCharOffset;
}

class LineRange {
  const LineRange(this.startLine, this.lineCount);

  final int startLine;
  final int lineCount;
}

class InlineStyle {
  const InlineStyle(this.foreground, this.background, this.fontAttributes);

  static const int styleBold = 1;
  static const int styleItalic = styleBold << 1;
  static const int styleStrikeThrough = styleItalic << 1;

  final int foreground;
  final int background;
  final int fontAttributes;

  bool get isBold => (fontAttributes & styleBold) != 0;
  bool get isItalic => (fontAttributes & styleItalic) != 0;
  bool get isStrikethrough => (fontAttributes & styleStrikeThrough) != 0;

  @override
  String toString() {
    return 'InlineStyle('
        'foreground: $foreground, '
        'background: $background, '
        'fontAttributes: $fontAttributes)';
  }
}

class TokenSpan {
  TokenSpan.styleId(this.range, this.styleId) : inlineStyle = null;

  TokenSpan.inlineStyle(this.range, this.inlineStyle) : styleId = null;

  final TextRange range;
  final int? styleId;
  final InlineStyle? inlineStyle;

  bool get usesInlineStyle => inlineStyle != null;

  @override
  String toString() {
    if (inlineStyle != null) {
      return 'TokenSpan(range: $range, inlineStyle: $inlineStyle)';
    }
    return 'TokenSpan(range: $range, styleId: $styleId)';
  }
}

class LineHighlight {
  LineHighlight([List<TokenSpan>? spans]) : spans = spans ?? <TokenSpan>[];

  final List<TokenSpan> spans;
}

class DocumentHighlight {
  DocumentHighlight([List<LineHighlight>? lines])
    : lines = lines ?? <LineHighlight>[];

  final List<LineHighlight> lines;
}

class DocumentHighlightSlice {
  DocumentHighlightSlice({
    this.startLine = 0,
    this.totalLineCount = 0,
    List<LineHighlight>? lines,
  }) : lines = lines ?? <LineHighlight>[];

  final int startLine;
  final int totalLineCount;
  final List<LineHighlight> lines;
}

enum BracketTokenKind { open, close }

enum BracketMatchState { matched, unmatched, unknown }

class BracketToken {
  const BracketToken({
    required this.range,
    required this.depth,
    required this.kind,
    required this.matchState,
    this.partnerRange,
  });

  final TextRange range;
  final int depth;
  final BracketTokenKind kind;
  final BracketMatchState matchState;
  final TextRange? partnerRange;
}

class LineBracketPairs {
  LineBracketPairs([List<BracketToken>? tokens])
    : tokens = tokens ?? <BracketToken>[];

  final List<BracketToken> tokens;
}

class BracketPairResult {
  BracketPairResult({
    this.startLine = 0,
    this.totalLineCount = 0,
    List<LineBracketPairs>? lines,
  }) : lines = lines ?? <LineBracketPairs>[];

  final int startLine;
  final int totalLineCount;
  final List<LineBracketPairs> lines;
}

class LineAnalyzeResult {
  const LineAnalyzeResult(this.highlight, this.endState, this.charCount);

  final LineHighlight highlight;
  final int endState;
  final int charCount;
}

class IndentGuideBranchPoint {
  const IndentGuideBranchPoint(this.line, this.column);

  final int line;
  final int column;
}

class IndentGuideLine {
  IndentGuideLine(
    this.column,
    this.startLine,
    this.endLine,
    this.continuesBefore,
    this.continuesAfter, [
    List<IndentGuideBranchPoint>? branches,
  ]) : branches = branches ?? <IndentGuideBranchPoint>[];

  final int column;
  final int startLine;
  final int endLine;
  final bool continuesBefore;
  final bool continuesAfter;
  final List<IndentGuideBranchPoint> branches;
}

class LineScopeState {
  const LineScopeState(
    this.nestingLevel,
    this.scopeState,
    this.scopeColumn,
    this.indentLevel,
  );

  static const int start = 0;
  static const int end = 1;
  static const int content = 2;

  final int nestingLevel;
  final int scopeState;
  final int scopeColumn;
  final int indentLevel;
}

class IndentGuideResult {
  IndentGuideResult({
    this.startLine = 0,
    List<IndentGuideLine>? guideLines,
    List<LineScopeState>? lineStates,
  }) : guideLines = guideLines ?? <IndentGuideLine>[],
       lineStates = lineStates ?? <LineScopeState>[];

  final int startLine;
  final List<IndentGuideLine> guideLines;
  final List<LineScopeState> lineStates;
}
