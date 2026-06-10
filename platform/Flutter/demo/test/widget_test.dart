import 'package:demo/code_view.dart';
import 'package:demo/highlight_theme.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:sweetline/sweetline.dart';

void main() {
  testWidgets('CodeView shows placeholder without data', (
    WidgetTester tester,
  ) async {
    await tester.pumpWidget(
      MaterialApp(
        home: Scaffold(
          body: CodeView(
            theme: HighlightTheme.sweetLineDark(),
            sourceCode: '',
            highlight: null,
            indentGuides: null,
            bracketPairs: null,
          ),
        ),
      ),
    );

    expect(find.text('Select a file to highlight'), findsOneWidget);
  });

  test('sweetline native asset can create engine', () {
    final engine = HighlightEngine();
    engine.close();
  });
}
