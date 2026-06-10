import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:sweetline/sweetline.dart';

import 'code_view.dart';
import 'generated/demo_assets.g.dart';
import 'highlight_theme.dart';

void main() {
  runApp(const SweetLineDemoApp());
}

class SweetLineDemoApp extends StatelessWidget {
  const SweetLineDemoApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'SweetLine Flutter Demo',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(useMaterial3: true, brightness: Brightness.dark),
      home: const DemoPage(),
    );
  }
}

class DemoPage extends StatefulWidget {
  const DemoPage({super.key});

  @override
  State<DemoPage> createState() => _DemoPageState();
}

class _DemoPageState extends State<DemoPage> {
  static final Map<String, DemoAssetEntry> _samplesByFileName =
      <String, DemoAssetEntry>{
        for (final sample in demoAssetEntries) sample.fileName: sample,
      };

  late final HighlightEngine _engine;
  late final List<HighlightTheme> _themes;
  late HighlightTheme _currentTheme;

  String _selectedFile = demoAssetFileNames.first;
  bool _isLoading = false;
  String? _error;
  String _statusText = 'Ready';
  String? _warmupSummary;
  String _sourceCode = '';
  DocumentHighlight? _highlight;
  IndentGuideResult? _indentGuides;
  BracketPairResult? _bracketPairs;

  @override
  void initState() {
    super.initState();
    _themes = HighlightTheme.builtinThemes();
    _currentTheme = _themes.first;
    _engine = HighlightEngine(
      const HighlightConfig(showIndex: true, inlineStyle: false),
    );
    _registerStyleNames(_engine);
    _definePlatformMacros(_engine);

    WidgetsBinding.instance.addPostFrameCallback((_) {
      _warmUpAndHighlightInitialSample();
    });
  }

  @override
  void dispose() {
    _engine.close();
    super.dispose();
  }

  Future<void> _highlightFile(String fileName) async {
    final sample = _samplesByFileName[fileName];
    if (sample == null) {
      setState(() {
        _selectedFile = fileName;
        _error = 'Unknown demo file: $fileName';
        _statusText = _error!;
      });
      return;
    }

    setState(() {
      _selectedFile = fileName;
      _isLoading = true;
      _error = null;
      _statusText = 'Loading $fileName...';
    });

    try {
      final sourceCode = await rootBundle.loadString(sample.sourceAssetPath);
      final document = Document(fileName, sourceCode);
      DocumentAnalyzer? analyzer;
      try {
        final loadWatch = Stopwatch()..start();
        analyzer = _engine.loadDocument(document);
        loadWatch.stop();
        if (analyzer == null) {
          throw StateError('No matching syntax for $fileName');
        }

        final analyzeWatch = Stopwatch()..start();
        final highlight = analyzer.analyze();
        final indentGuides = analyzer.analyzeIndentGuides();
        final bracketPairs = analyzer.analyzeBracketPairs();
        analyzeWatch.stop();

        if (!mounted) {
          return;
        }

        setState(() {
          _sourceCode = sourceCode;
          _highlight = highlight;
          _indentGuides = indentGuides;
          _bracketPairs = bracketPairs;
          _isLoading = false;
          _statusText = <String>[
            ...?_warmupSummary == null ? null : <String>[_warmupSummary!],
            'Load: ${loadWatch.elapsedMicroseconds}us',
            'Analyze: ${analyzeWatch.elapsedMicroseconds}us',
            'Lines: ${sourceCode.split('\n').length}',
            'File: $fileName',
          ].join(' | ');
        });
      } finally {
        analyzer?.close();
        document.close();
      }
    } catch (error) {
      if (!mounted) {
        return;
      }
      setState(() {
        _isLoading = false;
        _error = '$error';
        _statusText = 'Error: $_error';
      });
    }
  }

  Future<void> _warmUpAndHighlightInitialSample() async {
    setState(() {
      _isLoading = true;
      _error = null;
      _statusText =
          'Preparing ${demoCommonSyntaxAssetPaths.length} syntax rule files...';
    });

    try {
      final warmupWatch = Stopwatch()..start();
      final compiledCount = await _precompileCommonSyntaxes();
      warmupWatch.stop();

      if (!mounted) {
        return;
      }

      final warmupSummary =
          'Warmup: $compiledCount syntaxes in ${warmupWatch.elapsedMilliseconds} ms';
      setState(() {
        _warmupSummary = warmupSummary;
        _statusText =
            'Compiled $compiledCount syntax rule files in ${warmupWatch.elapsedMilliseconds} ms';
      });

      await _highlightFile(_selectedFile);
    } catch (error) {
      if (!mounted) {
        return;
      }
      setState(() {
        _isLoading = false;
        _error = '$error';
        _statusText = 'Error: $_error';
      });
    }
  }

  Future<int> _precompileCommonSyntaxes() async {
    final syntaxJsonByAssetPath = <String, String>{};
    for (final assetPath in demoCommonSyntaxAssetPaths) {
      syntaxJsonByAssetPath[assetPath] = await rootBundle.loadString(assetPath);
    }

    var pendingAssetPaths = syntaxJsonByAssetPath.keys.toList(growable: true)
      ..sort((left, right) => left.compareTo(right));
    var compiledCount = 0;

    while (pendingAssetPaths.isNotEmpty) {
      var progressed = false;
      final nextPending = <String>[];

      for (final assetPath in pendingAssetPaths) {
        try {
          _engine.compileSyntaxFromJson(syntaxJsonByAssetPath[assetPath]!);
          compiledCount += 1;
          progressed = true;
        } on SyntaxCompileError catch (error) {
          if (error.errorCode == SyntaxCompileError.errImportSyntaxNotFound) {
            nextPending.add(assetPath);
            continue;
          }
          throw StateError(
            'Failed to compile ${_assetFileName(assetPath)}: $error',
          );
        }
      }

      if (!progressed) {
        final unresolved = nextPending.map(_assetFileName).join(', ');
        throw StateError('Unresolved syntax dependencies: $unresolved');
      }

      pendingAssetPaths = nextPending;
    }

    return compiledCount;
  }

  String _assetFileName(String assetPath) {
    return assetPath.split('/').last;
  }

  @override
  Widget build(BuildContext context) {
    final theme = _currentTheme;
    return Scaffold(
      backgroundColor: theme.background,
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(14),
          child: Container(
            decoration: BoxDecoration(
              color: HighlightTheme.blend(
                theme.background,
                Colors.white,
                0.015,
              ),
              borderRadius: BorderRadius.circular(18),
              border: Border.all(color: theme.separator),
              boxShadow: <BoxShadow>[
                BoxShadow(
                  color: Colors.black.withAlpha(46),
                  blurRadius: 18,
                  offset: const Offset(0, 10),
                ),
              ],
            ),
            clipBehavior: Clip.antiAlias,
            child: Column(
              children: <Widget>[
                _HeaderBar(
                  theme: theme,
                  selectedFile: _selectedFile,
                  exampleFiles: demoAssetFileNames,
                  themeNames: _themes
                      .map((theme) => theme.name)
                      .toList(growable: false),
                  selectedThemeName: theme.name,
                  fileSelectionEnabled: !_isLoading,
                  onFileSelected: _highlightFile,
                  onThemeSelected: (themeName) {
                    final nextTheme = _themes.firstWhere(
                      (item) => item.name == themeName,
                    );
                    setState(() {
                      _currentTheme = nextTheme;
                    });
                  },
                ),
                Container(height: 1, color: theme.separator),
                Expanded(
                  child: Stack(
                    children: <Widget>[
                      Positioned.fill(
                        child: CodeView(
                          theme: theme,
                          sourceCode: _sourceCode,
                          highlight: _highlight,
                          indentGuides: _indentGuides,
                          bracketPairs: _bracketPairs,
                          placeholder: _isLoading
                              ? 'Analyzing...'
                              : (_error ?? 'Select a file'),
                        ),
                      ),
                      if (_isLoading)
                        const Positioned(
                          top: 18,
                          right: 18,
                          child: SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(strokeWidth: 2.2),
                          ),
                        ),
                    ],
                  ),
                ),
                Container(height: 1, color: theme.separator),
                _StatusBar(
                  theme: theme,
                  statusText: _statusText,
                  errorText: _error,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  static void _registerStyleNames(HighlightEngine engine) {
    engine.registerStyleName('keyword', HighlightTheme.styleKeyword);
    engine.registerStyleName('string', HighlightTheme.styleString);
    engine.registerStyleName('number', HighlightTheme.styleNumber);
    engine.registerStyleName('comment', HighlightTheme.styleComment);
    engine.registerStyleName('class', HighlightTheme.styleClass);
    engine.registerStyleName('method', HighlightTheme.styleMethod);
    engine.registerStyleName('variable', HighlightTheme.styleVariable);
    engine.registerStyleName('punctuation', HighlightTheme.stylePunctuation);
    engine.registerStyleName('annotation', HighlightTheme.styleAnnotation);
    engine.registerStyleName('preprocessor', HighlightTheme.stylePreprocessor);
    engine.registerStyleName('macro', HighlightTheme.styleMacro);
    engine.registerStyleName('lifetime', HighlightTheme.styleLifetime);
    engine.registerStyleName('selector', HighlightTheme.styleSelector);
    engine.registerStyleName('builtin', HighlightTheme.styleBuiltin);
    engine.registerStyleName('url', HighlightTheme.styleUrl);
    engine.registerStyleName('property', HighlightTheme.styleProperty);
  }

  static void _definePlatformMacros(HighlightEngine engine) {
    if (kIsWeb) {
      return;
    }
    if (Platform.isAndroid) {
      engine.defineMacro('ANDROID');
    } else if (Platform.isWindows) {
      engine.defineMacro('WINDOWS');
    } else if (Platform.isMacOS) {
      engine.defineMacro('MACOS');
    } else if (Platform.isIOS) {
      engine.defineMacro('IOS');
    } else if (Platform.isLinux) {
      engine.defineMacro('LINUX');
    }
  }
}

class _HeaderBar extends StatelessWidget {
  const _HeaderBar({
    required this.theme,
    required this.selectedFile,
    required this.exampleFiles,
    required this.themeNames,
    required this.selectedThemeName,
    required this.fileSelectionEnabled,
    required this.onFileSelected,
    required this.onThemeSelected,
  });

  final HighlightTheme theme;
  final String selectedFile;
  final List<String> exampleFiles;
  final List<String> themeNames;
  final String selectedThemeName;
  final bool fileSelectionEnabled;
  final ValueChanged<String> onFileSelected;
  final ValueChanged<String> onThemeSelected;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.fromLTRB(18, 16, 18, 16),
      decoration: BoxDecoration(color: theme.toolbarSurface),
      child: Wrap(
        crossAxisAlignment: WrapCrossAlignment.center,
        spacing: 14,
        runSpacing: 12,
        children: <Widget>[
          Text(
            'SweetLine Flutter Demo',
            style: TextStyle(
              color: theme.text,
              fontSize: 20,
              fontWeight: FontWeight.w700,
              letterSpacing: 0.2,
            ),
          ),
          _LabeledDropdown(
            label: 'File',
            value: selectedFile,
            items: exampleFiles,
            theme: theme,
            onChanged: fileSelectionEnabled ? onFileSelected : null,
            width: 220,
          ),
          _LabeledDropdown(
            label: 'Theme',
            value: selectedThemeName,
            items: themeNames,
            theme: theme,
            onChanged: onThemeSelected,
            width: 180,
          ),
        ],
      ),
    );
  }
}

class _LabeledDropdown extends StatelessWidget {
  const _LabeledDropdown({
    required this.label,
    required this.value,
    required this.items,
    required this.theme,
    required this.onChanged,
    required this.width,
  });

  final String label;
  final String value;
  final List<String> items;
  final HighlightTheme theme;
  final ValueChanged<String>? onChanged;
  final double width;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: width,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: <Widget>[
          Text(
            label.toUpperCase(),
            style: TextStyle(
              color: theme.lineNumber,
              fontSize: 11,
              fontWeight: FontWeight.w700,
              letterSpacing: 1.1,
            ),
          ),
          const SizedBox(height: 6),
          DecoratedBox(
            decoration: BoxDecoration(
              color: HighlightTheme.blend(theme.background, Colors.white, 0.03),
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: theme.separator),
            ),
            child: DropdownButtonHideUnderline(
              child: DropdownButton<String>(
                value: value,
                isExpanded: true,
                dropdownColor: theme.toolbarSurface,
                padding: const EdgeInsets.symmetric(horizontal: 12),
                borderRadius: BorderRadius.circular(12),
                iconEnabledColor: theme.accent,
                style: TextStyle(
                  color: theme.text,
                  fontSize: 13,
                  fontWeight: FontWeight.w500,
                ),
                items: items
                    .map(
                      (item) => DropdownMenuItem<String>(
                        value: item,
                        child: Text(item, overflow: TextOverflow.ellipsis),
                      ),
                    )
                    .toList(growable: false),
                onChanged: (value) {
                  if (value != null && onChanged != null) {
                    onChanged!.call(value);
                  }
                },
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _StatusBar extends StatelessWidget {
  const _StatusBar({
    required this.theme,
    required this.statusText,
    required this.errorText,
  });

  final HighlightTheme theme;
  final String statusText;
  final String? errorText;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      decoration: BoxDecoration(color: theme.statusSurface),
      child: Text(
        statusText,
        style: TextStyle(
          color: errorText == null ? theme.lineNumber : const Color(0xFFFF8A7A),
          fontSize: 12,
          fontWeight: FontWeight.w500,
          letterSpacing: 0.15,
        ),
      ),
    );
  }
}
