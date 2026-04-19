import 'dart:io';

const Set<String> _excludedSyntaxFiles = <String>{'yaml(non zero width).json'};
const Set<String> _exactNameDemoSamples = <String>{'meson.build', 'Justfile'};

const String _syntaxSampleName = 'json-sweetline.json';

void main() {
  final scriptFile = File.fromUri(Platform.script).absolute;
  final demoRoot = scriptFile.parent.parent;
  final flutterRoot = demoRoot.parent;
  final repoRoot = flutterRoot.parent.parent;

  final syntaxesSourceDir = Directory(
    '${repoRoot.path}${Platform.pathSeparator}syntaxes',
  );
  final examplesSourceDir = Directory(
    '${repoRoot.path}${Platform.pathSeparator}tests${Platform.pathSeparator}files',
  );
  final syntaxAssetsDir = Directory(
    '${demoRoot.path}${Platform.pathSeparator}assets${Platform.pathSeparator}syntaxes',
  );
  final exampleAssetsDir = Directory(
    '${demoRoot.path}${Platform.pathSeparator}assets${Platform.pathSeparator}examples',
  );
  final generatedFile = File(
    '${demoRoot.path}${Platform.pathSeparator}lib${Platform.pathSeparator}generated${Platform.pathSeparator}demo_assets.g.dart',
  );

  if (!syntaxesSourceDir.existsSync()) {
    throw StateError(
      'Syntax source directory not found: ${syntaxesSourceDir.path}',
    );
  }
  if (!examplesSourceDir.existsSync()) {
    throw StateError(
      'Example source directory not found: ${examplesSourceDir.path}',
    );
  }

  final syntaxFiles = _listFiles(syntaxesSourceDir, _isCommonSyntaxFile);
  final exampleFiles = _listFiles(examplesSourceDir, _isDemoExampleFile);

  _syncDirectory(sourceFiles: syntaxFiles, destinationDir: syntaxAssetsDir);
  _syncDirectory(sourceFiles: exampleFiles, destinationDir: exampleAssetsDir);

  final syntaxNames = syntaxFiles
      .map((file) => file.uri.pathSegments.last)
      .toSet();
  final syntaxAssetPaths = syntaxFiles
      .map((file) => 'assets/syntaxes/${file.uri.pathSegments.last}')
      .toList(growable: false);
  final exampleSamples =
      exampleFiles.map(_buildExampleSample).toList(growable: true)
        ..sort((left, right) => left.fileName.compareTo(right.fileName));

  if (!syntaxNames.contains(_syntaxSampleName)) {
    throw StateError('Missing syntax sample: $_syntaxSampleName');
  }
  exampleSamples.add(
    const _DemoSampleData(
      fileName: _syntaxSampleName,
      sourceAssetPath: 'assets/syntaxes/json-sweetline.json',
    ),
  );

  generatedFile.parent.createSync(recursive: true);
  generatedFile.writeAsStringSync(
    _buildGeneratedSource(
      samples: exampleSamples,
      syntaxAssetPaths: syntaxAssetPaths,
    ),
  );

  stdout.writeln(
    'Synced ${syntaxFiles.length} common syntaxes and ${exampleFiles.length} examples.',
  );
  stdout.writeln('Generated asset manifest: ${generatedFile.path}');
}

List<File> _listFiles(Directory directory, bool Function(File file) predicate) {
  final files =
      directory
          .listSync()
          .whereType<File>()
          .where(predicate)
          .toList(growable: false)
        ..sort((left, right) => left.path.compareTo(right.path));
  return files;
}

void _syncDirectory({
  required List<File> sourceFiles,
  required Directory destinationDir,
}) {
  destinationDir.createSync(recursive: true);

  final expectedNames = <String>{};
  for (final sourceFile in sourceFiles) {
    final fileName = sourceFile.uri.pathSegments.last;
    expectedNames.add(fileName);
    final destinationPath =
        '${destinationDir.path}${Platform.pathSeparator}$fileName';
    sourceFile.copySync(destinationPath);
  }

  for (final entity in destinationDir.listSync()) {
    if (entity is! File) {
      continue;
    }
    final fileName = entity.uri.pathSegments.last;
    if (!expectedNames.contains(fileName)) {
      entity.deleteSync();
    }
  }
}

_DemoSampleData _buildExampleSample(File file) {
  final fileName = file.uri.pathSegments.last;
  return _DemoSampleData(
    fileName: fileName,
    sourceAssetPath: 'assets/examples/$fileName',
  );
}

bool _isCommonSyntaxFile(File file) {
  final fileName = file.uri.pathSegments.last;
  if (!fileName.toLowerCase().endsWith('.json')) {
    return false;
  }
  if (_excludedSyntaxFiles.contains(fileName)) {
    return false;
  }
  return !fileName.endsWith('-inlineStyle.json');
}

bool _isDemoExampleFile(File file) {
  final fileName = file.uri.pathSegments.last;
  return fileName.startsWith('example') || _exactNameDemoSamples.contains(fileName);
}

String _buildGeneratedSource({
  required List<_DemoSampleData> samples,
  required List<String> syntaxAssetPaths,
}) {
  final buffer = StringBuffer()
    ..writeln('// GENERATED CODE - DO NOT MODIFY BY HAND.')
    ..writeln('// Run: dart run tool/sync_demo_assets.dart')
    ..writeln()
    ..writeln('class DemoAssetEntry {')
    ..writeln('  const DemoAssetEntry({')
    ..writeln('    required this.fileName,')
    ..writeln('    required this.sourceAssetPath,')
    ..writeln('  });')
    ..writeln()
    ..writeln('  final String fileName;')
    ..writeln('  final String sourceAssetPath;')
    ..writeln('}')
    ..writeln()
    ..writeln(
      'const List<DemoAssetEntry> demoAssetEntries = <DemoAssetEntry>[',
    );

  for (final sample in samples) {
    buffer
      ..writeln('  DemoAssetEntry(')
      ..writeln("    fileName: '${_escapeDartString(sample.fileName)}',")
      ..writeln(
        "    sourceAssetPath: '${_escapeDartString(sample.sourceAssetPath)}',",
      )
      ..writeln('  ),');
  }

  buffer
    ..writeln('];')
    ..writeln()
    ..writeln('const List<String> demoCommonSyntaxAssetPaths = <String>[');

  for (final syntaxAssetPath in syntaxAssetPaths) {
    buffer.writeln("  '${_escapeDartString(syntaxAssetPath)}',");
  }

  buffer
    ..writeln('];')
    ..writeln()
    ..writeln('const List<String> demoAssetFileNames = <String>[');

  for (final sample in samples) {
    buffer.writeln("  '${_escapeDartString(sample.fileName)}',");
  }

  buffer.writeln('];');
  return buffer.toString();
}

String _escapeDartString(String value) {
  return value.replaceAll(r'\', r'\\').replaceAll("'", r"\'");
}

class _DemoSampleData {
  const _DemoSampleData({
    required this.fileName,
    required this.sourceAssetPath,
  });

  final String fileName;
  final String sourceAssetPath;
}
