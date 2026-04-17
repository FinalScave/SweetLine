import 'dart:io';

const Map<String, String> _extensionToSyntax = <String, String>{
  '.t': 'tiecode.json',
  '.c': 'c.json',
  '.cpp': 'cpp.json',
  '.cs': 'csharp.json',
  '.dart': 'dart.json',
  '.go': 'go.json',
  '.groovy': 'groovy.json',
  '.html': 'html.json',
  '.java': 'java.json',
  '.js': 'javascript.json',
  '.json': 'json-sweetline.json',
  '.jsonc': 'jsonc.json',
  '.json5': 'json5.json',
  '.kt': 'kotlin.json',
  '.lua': 'lua.json',
  '.m': 'objc.json',
  '.php': 'php.json',
  '.ps1': 'powershell.json',
  '.py': 'python.json',
  '.rs': 'rust.json',
  '.scala': 'scala.json',
  '.sh': 'shell.json',
  '.sql': 'sql.json',
  '.swift': 'swift.json',
  '.toml': 'toml.json',
  '.ts': 'typescript.json',
  '.vb': 'vb.json',
  '.xml': 'xml.json',
  '.yaml': 'yaml.json',
  '.md': 'markdown.json',
  '.wenyan': 'wenyan.json',
  '.myu': 'iapp.json',
  '.css': 'css.json',
  '.scss': 'scss.json',
  '.less': 'less.json',
  '.cmake': 'cmake.json',
  '.dockerfile': 'dockerfile.json',
  '.mk': 'makefile.json',
  '.properties': 'properties.json',
  '.env': 'env.json',
  '.proto': 'protobuf.json',
  '.graphql': 'graphql.json',
  '.gql': 'graphql.json',
  '.nginx': 'nginx.json',
  '.conf': 'nginx.json',
  '.gitignore': 'gitignore.json',
  '.diff': 'diff.json',
  '.patch': 'diff.json',
  '.rb': 'ruby.json',
  '.rake': 'ruby.json',
  '.gemspec': 'ruby.json',
  '.ru': 'ruby.json',
  '.hcl': 'hcl.json',
  '.tf': 'terraform.json',
  '.tfvars': 'terraform.json',
  '.tfbackend': 'terraform.json',
  '.vue': 'vue.json',
  '.svelte': 'svelte.json',
};

const String _syntaxSampleName = 'json-sweetline.json';

void main() {
  final scriptFile = File.fromUri(Platform.script).absolute;
  // demo/tool/sync_demo_assets.dart → demo/tool/ → demo/
  final demoRoot = scriptFile.parent.parent;
  // demo/ → Flutter/
  final flutterRoot = demoRoot.parent;
  // Flutter/ → platform/ → SweetLine/
  final repoRoot = flutterRoot.parent.parent;

  final syntaxesSourceDir = Directory('${repoRoot.path}${Platform.pathSeparator}syntaxes');
  final examplesSourceDir = Directory('${repoRoot.path}${Platform.pathSeparator}tests${Platform.pathSeparator}files');
  final syntaxAssetsDir = Directory('${demoRoot.path}${Platform.pathSeparator}assets${Platform.pathSeparator}syntaxes');
  final exampleAssetsDir = Directory(
    '${demoRoot.path}${Platform.pathSeparator}assets${Platform.pathSeparator}examples',
  );
  final generatedFile = File(
    '${demoRoot.path}${Platform.pathSeparator}lib${Platform.pathSeparator}generated${Platform.pathSeparator}demo_assets.g.dart',
  );

  if (!syntaxesSourceDir.existsSync()) {
    throw StateError('Syntax source directory not found: ${syntaxesSourceDir.path}');
  }
  if (!examplesSourceDir.existsSync()) {
    throw StateError('Example source directory not found: ${examplesSourceDir.path}');
  }

  final syntaxFiles = _listFiles(syntaxesSourceDir, (file) => file.path.toLowerCase().endsWith('.json'));
  final exampleFiles = _listFiles(examplesSourceDir, (file) => file.uri.pathSegments.last.startsWith('example.'));

  _syncDirectory(sourceFiles: syntaxFiles, destinationDir: syntaxAssetsDir);
  _syncDirectory(sourceFiles: exampleFiles, destinationDir: exampleAssetsDir);

  final syntaxNames = syntaxFiles.map((file) => file.uri.pathSegments.last).toSet();
  final exampleSamples = exampleFiles.map(_buildExampleSample).toList(growable: true)
    ..sort((left, right) => left.fileName.compareTo(right.fileName));

  if (!syntaxNames.contains(_syntaxSampleName)) {
    throw StateError('Missing syntax sample: $_syntaxSampleName');
  }
  exampleSamples.add(
    const _DemoSampleData(
      fileName: _syntaxSampleName,
      sourceAssetPath: 'assets/syntaxes/json-sweetline.json',
      syntaxAssetPath: 'assets/syntaxes/json-sweetline.json',
    ),
  );

  for (final sample in exampleSamples) {
    final syntaxFileName = sample.syntaxAssetPath.split('/').last;
    if (!syntaxNames.contains(syntaxFileName)) {
      throw StateError('Missing syntax asset "$syntaxFileName" for sample "${sample.fileName}"');
    }
  }

  generatedFile.parent.createSync(recursive: true);
  generatedFile.writeAsStringSync(_buildGeneratedSource(exampleSamples));

  stdout.writeln('Synced ${syntaxFiles.length} syntaxes and ${exampleFiles.length} examples.');
  stdout.writeln('Generated asset manifest: ${generatedFile.path}');
}

List<File> _listFiles(Directory directory, bool Function(File file) predicate) {
  final files = directory.listSync().whereType<File>().where(predicate).toList(growable: false)
    ..sort((left, right) => left.path.compareTo(right.path));
  return files;
}

void _syncDirectory({required List<File> sourceFiles, required Directory destinationDir}) {
  destinationDir.createSync(recursive: true);

  final expectedNames = <String>{};
  for (final sourceFile in sourceFiles) {
    final fileName = sourceFile.uri.pathSegments.last;
    expectedNames.add(fileName);
    final destinationPath = '${destinationDir.path}${Platform.pathSeparator}$fileName';
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
  final extensionIndex = fileName.indexOf('.');
  final extension = extensionIndex >= 0 ? fileName.substring(extensionIndex) : '';
  final syntaxFileName = _extensionToSyntax[extension];
  if (syntaxFileName == null) {
    throw StateError('No syntax mapping for example file: $fileName');
  }
  return _DemoSampleData(
    fileName: fileName,
    sourceAssetPath: 'assets/examples/$fileName',
    syntaxAssetPath: 'assets/syntaxes/$syntaxFileName',
  );
}

String _buildGeneratedSource(List<_DemoSampleData> samples) {
  final buffer = StringBuffer()
    ..writeln('// GENERATED CODE - DO NOT MODIFY BY HAND.')
    ..writeln('// Run: dart run tool/sync_demo_assets.dart')
    ..writeln()
    ..writeln('class DemoAssetEntry {')
    ..writeln('  const DemoAssetEntry({')
    ..writeln('    required this.fileName,')
    ..writeln('    required this.sourceAssetPath,')
    ..writeln('    required this.syntaxAssetPath,')
    ..writeln('  });')
    ..writeln()
    ..writeln('  final String fileName;')
    ..writeln('  final String sourceAssetPath;')
    ..writeln('  final String syntaxAssetPath;')
    ..writeln('}')
    ..writeln()
    ..writeln('const List<DemoAssetEntry> demoAssetEntries = <DemoAssetEntry>[');

  for (final sample in samples) {
    buffer
      ..writeln('  DemoAssetEntry(')
      ..writeln("    fileName: '${_escapeDartString(sample.fileName)}',")
      ..writeln("    sourceAssetPath: '${_escapeDartString(sample.sourceAssetPath)}',")
      ..writeln("    syntaxAssetPath: '${_escapeDartString(sample.syntaxAssetPath)}',")
      ..writeln('  ),');
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
  const _DemoSampleData({required this.fileName, required this.sourceAssetPath, required this.syntaxAssetPath});

  final String fileName;
  final String sourceAssetPath;
  final String syntaxAssetPath;
}
