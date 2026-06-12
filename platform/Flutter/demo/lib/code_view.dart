import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:sweetline/sweetline.dart';

import 'highlight_theme.dart';

class CodeView extends StatefulWidget {
  const CodeView({
    super.key,
    required this.theme,
    required this.sourceCode,
    required this.highlight,
    required this.indentGuides,
    required this.bracketPairs,
    this.placeholder = 'Select a file to highlight',
  });

  final HighlightTheme theme;
  final String sourceCode;
  final DocumentHighlight? highlight;
  final IndentGuideResult? indentGuides;
  final BracketPairResult? bracketPairs;
  final String placeholder;

  @override
  State<CodeView> createState() => _CodeViewState();
}

class _CodeViewState extends State<CodeView> {
  late final ScrollController _verticalController;
  late final ScrollController _horizontalController;

  @override
  void initState() {
    super.initState();
    _verticalController = ScrollController();
    _horizontalController = ScrollController();
  }

  @override
  void dispose() {
    _verticalController.dispose();
    _horizontalController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (widget.sourceCode.isEmpty || widget.highlight == null) {
      return DecoratedBox(
        decoration: BoxDecoration(color: widget.theme.background),
        child: Center(
          child: Text(
            widget.placeholder,
            style: TextStyle(
              color: widget.theme.lineNumber,
              fontSize: 15,
              letterSpacing: 0.2,
            ),
          ),
        ),
      );
    }

    return LayoutBuilder(
      builder: (context, constraints) {
        final metrics = _CodeViewMetrics.measure(widget.sourceCode);
        final canvasWidth = math.max(metrics.width, constraints.maxWidth);
        final canvasHeight = math.max(metrics.height, constraints.maxHeight);

        final painter = _CodeViewPainter(
          theme: widget.theme,
          sourceCode: widget.sourceCode,
          highlight: widget.highlight!,
          indentGuides: widget.indentGuides,
          bracketPairs: widget.bracketPairs,
          metrics: metrics,
        );

        return Scrollbar(
          thumbVisibility: true,
          controller: _verticalController,
          child: SingleChildScrollView(
            controller: _verticalController,
            primary: false,
            child: Scrollbar(
              thumbVisibility: true,
              controller: _horizontalController,
              notificationPredicate: (notification) =>
                  notification.metrics.axis == Axis.horizontal,
              child: SingleChildScrollView(
                controller: _horizontalController,
                scrollDirection: Axis.horizontal,
                child: CustomPaint(
                  size: Size(canvasWidth, canvasHeight),
                  painter: painter,
                ),
              ),
            ),
          ),
        );
      },
    );
  }
}

class _CodeViewMetrics {
  const _CodeViewMetrics({
    required this.textStyle,
    required this.lineNumberStyle,
    required this.lineHeight,
    required this.charWidth,
    required this.gutterWidth,
    required this.codeLeftPadding,
    required this.verticalPadding,
    required this.width,
    required this.height,
    required this.lines,
  });

  static const double _fontSize = 14;
  static const double _lineNumberPadding = 14;
  static const double _codeLeftPadding = 12;
  static const double _verticalPadding = 14;

  final TextStyle textStyle;
  final TextStyle lineNumberStyle;
  final double lineHeight;
  final double charWidth;
  final double gutterWidth;
  final double codeLeftPadding;
  final double verticalPadding;
  final double width;
  final double height;
  final List<String> lines;

  static _CodeViewMetrics measure(String sourceCode) {
    final lines = sourceCode.split('\n');
    const textStyle = TextStyle(
      fontSize: _fontSize,
      fontFamily: 'monospace',
      height: 1.45,
    );
    final lineNumberStyle = textStyle.copyWith(fontSize: 12);

    final samplePainter = TextPainter(
      text: const TextSpan(text: '0', style: textStyle),
      textDirection: TextDirection.ltr,
      maxLines: 1,
    )..layout();
    final charWidth = samplePainter.width;
    final lineHeight = samplePainter.height;
    final gutterWidth =
        (lines.length.toString().length * charWidth) + (_lineNumberPadding * 2);

    var maxLineWidth = 0.0;
    for (final line in lines) {
      final painter = TextPainter(
        text: TextSpan(text: line.isEmpty ? ' ' : line, style: textStyle),
        textDirection: TextDirection.ltr,
        maxLines: 1,
      )..layout();
      maxLineWidth = math.max(maxLineWidth, painter.width);
    }

    final width = gutterWidth + _codeLeftPadding + maxLineWidth + 28;
    final height = (lines.length * lineHeight) + (_verticalPadding * 2);

    return _CodeViewMetrics(
      textStyle: textStyle,
      lineNumberStyle: lineNumberStyle,
      lineHeight: lineHeight,
      charWidth: charWidth,
      gutterWidth: gutterWidth,
      codeLeftPadding: _codeLeftPadding,
      verticalPadding: _verticalPadding,
      width: width,
      height: height,
      lines: lines,
    );
  }
}

class _CodeViewPainter extends CustomPainter {
  _CodeViewPainter({
    required this.theme,
    required this.sourceCode,
    required this.highlight,
    required this.indentGuides,
    required this.bracketPairs,
    required this.metrics,
  });

  final HighlightTheme theme;
  final String sourceCode;
  final DocumentHighlight highlight;
  final IndentGuideResult? indentGuides;
  final BracketPairResult? bracketPairs;
  final _CodeViewMetrics metrics;
  static const List<Color> _bracketPalette = <Color>[
    Color(0xFF7DD3FC),
    Color(0xFFF9A8D4),
    Color(0xFFFDE047),
    Color(0xFF86EFAC),
    Color(0xFFC4B5FD),
    Color(0xFFFDBA74),
  ];

  @override
  void paint(Canvas canvas, Size size) {
    final backgroundPaint = Paint()..color = theme.background;
    canvas.drawRect(Offset.zero & size, backgroundPaint);

    final gutterPaint = Paint()..color = theme.gutterBackground;
    canvas.drawRect(
      Rect.fromLTWH(0, 0, metrics.gutterWidth, size.height),
      gutterPaint,
    );

    final separatorPaint = Paint()
      ..color = theme.separator
      ..strokeWidth = 1;
    canvas.drawLine(
      Offset(metrics.gutterWidth, 0),
      Offset(metrics.gutterWidth, size.height),
      separatorPaint,
    );

    final codeOriginX = metrics.gutterWidth + metrics.codeLeftPadding;
    _drawIndentGuides(canvas, codeOriginX);
    _drawLineNumbers(canvas);
    _drawCode(canvas, codeOriginX);
  }

  void _drawLineNumbers(Canvas canvas) {
    for (var lineIndex = 0; lineIndex < metrics.lines.length; lineIndex++) {
      final y = metrics.verticalPadding + (lineIndex * metrics.lineHeight);
      final painter = TextPainter(
        text: TextSpan(
          text: '${lineIndex + 1}',
          style: metrics.lineNumberStyle.copyWith(color: theme.lineNumber),
        ),
        textDirection: TextDirection.ltr,
        maxLines: 1,
      )..layout();
      final x = metrics.gutterWidth - 14 - painter.width;
      painter.paint(
        canvas,
        Offset(x, y + ((metrics.lineHeight - painter.height) / 2)),
      );
    }
  }

  void _drawIndentGuides(Canvas canvas, double codeOriginX) {
    final guides = indentGuides?.guideLines;
    if (guides == null || guides.isEmpty) {
      return;
    }

    final guidePaint = Paint()
      ..color = theme.indentGuide
      ..strokeWidth = 1;

    for (final guide in guides) {
      final innerStart = guide.startLine + 1;
      final innerEnd = guide.endLine - 1;
      if (innerStart > innerEnd) {
        continue;
      }

      final x = codeOriginX + (guide.column * metrics.charWidth);
      final y1 = metrics.verticalPadding + (innerStart * metrics.lineHeight);
      final y2 =
          metrics.verticalPadding + ((innerEnd + 1) * metrics.lineHeight);
      _drawDashedLine(canvas, Offset(x, y1), Offset(x, y2), guidePaint);
    }
  }

  void _drawCode(Canvas canvas, double codeOriginX) {
    for (var lineIndex = 0; lineIndex < metrics.lines.length; lineIndex++) {
      final lineText = metrics.lines[lineIndex];
      final spans = lineIndex < highlight.lines.length
          ? highlight.lines[lineIndex].spans
          : const <TokenSpan>[];
      final lineBrackets = _bracketLine(lineIndex);

      var x = codeOriginX;
      final y = metrics.verticalPadding + (lineIndex * metrics.lineHeight);
      var lastColumn = 0;

      for (final span in spans) {
        final startColumn = span.range.start.column;
        final endColumn = span.range.start.line == span.range.end.line
            ? math.min(span.range.end.column, lineText.length)
            : lineText.length;

        if (startColumn > lastColumn && lastColumn < lineText.length) {
          x = _paintTextSegmentWithBrackets(
            canvas,
            lineText,
            lastColumn,
            math.min(startColumn, lineText.length),
            x,
            y,
            metrics.textStyle.copyWith(color: theme.text),
            lineBrackets,
          );
        }

        if (startColumn < lineText.length && endColumn > startColumn) {
          final segmentStyle = _segmentTextStyle(span);
          final beforeX = x;
          x = _paintTextSegmentWithBrackets(
            canvas,
            lineText,
            startColumn,
            endColumn,
            x,
            y,
            segmentStyle,
            lineBrackets,
          );

          if (span.inlineStyle?.isStrikethrough ?? false) {
            final strikeY = y + (metrics.lineHeight * 0.58);
            canvas.drawLine(
              Offset(beforeX, strikeY),
              Offset(x, strikeY),
              Paint()
                ..color = segmentStyle.color ?? theme.text
                ..strokeWidth = 1.1,
            );
          }
        }

        lastColumn = endColumn;
      }

      if (lastColumn < lineText.length) {
        _paintTextSegmentWithBrackets(
          canvas,
          lineText,
          lastColumn,
          lineText.length,
          x,
          y,
          metrics.textStyle.copyWith(color: theme.text),
          lineBrackets,
        );
      }
    }
  }

  LineBracketPairs? _bracketLine(int lineIndex) {
    final lines = bracketPairs?.lines;
    if (lines == null || lines.isEmpty) {
      return null;
    }
    final bracketLineIndex = lineIndex - (bracketPairs?.startLine ?? 0);
    return bracketLineIndex >= 0 && bracketLineIndex < lines.length
        ? lines[bracketLineIndex]
        : null;
  }

  Color _bracketColor(BracketToken token) {
    if (token.matchState == BracketMatchState.unmatched) {
      return const Color(0xFFFF6B6B);
    }
    final color = _bracketPalette[token.depth.abs() % _bracketPalette.length];
    if (token.matchState == BracketMatchState.unknown) {
      return color.withAlpha(170);
    }
    return color;
  }

  TextStyle _segmentTextStyle(TokenSpan span) {
    if (span.inlineStyle != null) {
      final inlineStyle = span.inlineStyle!;
      var fontWeight = FontWeight.w400;
      var fontStyle = FontStyle.normal;
      if (inlineStyle.isBold) {
        fontWeight = FontWeight.w700;
      }
      if (inlineStyle.isItalic) {
        fontStyle = FontStyle.italic;
      }
      return metrics.textStyle.copyWith(
        color: Color(
          inlineStyle.foreground == 0
              ? theme.textColor
              : inlineStyle.foreground,
        ),
        fontWeight: fontWeight,
        fontStyle: fontStyle,
      );
    }

    return metrics.textStyle.copyWith(
      color: Color(theme.getColor(span.styleId)),
    );
  }

  double _paintTextSegment(
    Canvas canvas,
    String text,
    double x,
    double y,
    TextStyle style,
  ) {
    if (text.isEmpty) {
      return x;
    }
    final painter = TextPainter(
      text: TextSpan(text: text, style: style),
      textDirection: TextDirection.ltr,
      maxLines: 1,
    )..layout();
    painter.paint(
      canvas,
      Offset(x, y + ((metrics.lineHeight - painter.height) / 2)),
    );
    return x + painter.width;
  }

  double _paintTextSegmentWithBrackets(
    Canvas canvas,
    String lineText,
    int startColumn,
    int endColumn,
    double x,
    double y,
    TextStyle style,
    LineBracketPairs? lineBrackets,
  ) {
    final start = startColumn.clamp(0, lineText.length).toInt();
    final end = endColumn.clamp(start, lineText.length).toInt();
    if (end <= start) {
      return x;
    }

    final tokens = lineBrackets?.tokens;
    if (tokens == null || tokens.isEmpty) {
      return _paintTextSegment(
        canvas,
        lineText.substring(start, end),
        x,
        y,
        style,
      );
    }

    var cursor = start;
    for (final token in tokens) {
      final tokenStart = token.range.start.column
          .clamp(0, lineText.length)
          .toInt();
      final tokenEnd = token.range.end.column
          .clamp(tokenStart, lineText.length)
          .toInt();
      if (tokenEnd <= cursor || tokenStart >= end) {
        continue;
      }

      final clippedStart = math.max(cursor, math.max(start, tokenStart));
      final clippedEnd = math.min(end, tokenEnd);
      if (clippedStart > cursor) {
        x = _paintTextSegment(
          canvas,
          lineText.substring(cursor, clippedStart),
          x,
          y,
          style,
        );
      }
      if (clippedEnd > clippedStart) {
        x = _paintTextSegment(
          canvas,
          lineText.substring(clippedStart, clippedEnd),
          x,
          y,
          style.copyWith(color: _bracketColor(token)),
        );
      }
      cursor = math.max(cursor, clippedEnd);
    }

    if (cursor < end) {
      x = _paintTextSegment(
        canvas,
        lineText.substring(cursor, end),
        x,
        y,
        style,
      );
    }
    return x;
  }

  void _drawDashedLine(Canvas canvas, Offset start, Offset end, Paint paint) {
    const dashLength = 3.0;
    const gapLength = 3.0;
    final distance = (end.dy - start.dy).abs();
    var drawn = 0.0;
    while (drawn < distance) {
      final from = start.dy + drawn;
      final to = math.min(from + dashLength, end.dy);
      canvas.drawLine(Offset(start.dx, from), Offset(end.dx, to), paint);
      drawn += dashLength + gapLength;
    }
  }

  @override
  bool shouldRepaint(covariant _CodeViewPainter oldDelegate) {
    return oldDelegate.theme != theme ||
        oldDelegate.sourceCode != sourceCode ||
        oldDelegate.highlight != highlight ||
        oldDelegate.indentGuides != indentGuides ||
        oldDelegate.bracketPairs != bracketPairs;
  }
}
