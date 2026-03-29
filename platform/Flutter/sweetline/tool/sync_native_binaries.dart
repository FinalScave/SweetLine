import 'dart:io';

const List<String> _supportedTargets = <String>[
  'android-arm64',
  'android-x64',
  'ios-arm64',
  'osx-arm64',
  'windows-x64',
];

void main() {
  final scriptFile = File.fromUri(Platform.script).absolute;
  final packageRoot = scriptFile.parent.parent;
  final repoRoot = packageRoot.parent.parent.parent;
  final prebuiltRoot = Directory(
    '${repoRoot.path}${Platform.pathSeparator}prebuilt',
  );
  final nativeRoot = Directory(
    '${packageRoot.path}${Platform.pathSeparator}native',
  );

  if (!prebuiltRoot.existsSync()) {
    throw StateError('Prebuilt directory not found: ${prebuiltRoot.path}');
  }

  if (nativeRoot.existsSync()) {
    nativeRoot.deleteSync(recursive: true);
  }
  nativeRoot.createSync(recursive: true);

  final copiedTargets = <String>[];
  for (final target in _supportedTargets) {
    final sourceDir = Directory(
      '${prebuiltRoot.path}${Platform.pathSeparator}$target',
    );
    if (!sourceDir.existsSync()) {
      continue;
    }
    final copiedCount = _copyDirectoryContents(
      sourceDir,
      Directory('${nativeRoot.path}${Platform.pathSeparator}$target'),
    );
    if (copiedCount > 0) {
      copiedTargets.add('$target ($copiedCount files)');
    }
  }

  if (copiedTargets.isEmpty) {
    stdout.writeln('No native binaries were copied from ${prebuiltRoot.path}.');
    return;
  }

  stdout.writeln('Synced native binaries to ${nativeRoot.path}:');
  for (final target in copiedTargets) {
    stdout.writeln('  - $target');
  }
}

int _copyDirectoryContents(Directory sourceDir, Directory destinationDir) {
  var copiedFiles = 0;
  for (final entity in sourceDir.listSync(followLinks: false)) {
    final name = _entityName(entity.path);
    if (name.isEmpty || name.startsWith('.')) {
      continue;
    }

    final destinationPath =
        '${destinationDir.path}${Platform.pathSeparator}$name';
    if (entity is File) {
      destinationDir.createSync(recursive: true);
      entity.copySync(destinationPath);
      copiedFiles++;
    } else if (entity is Directory) {
      copiedFiles += _copyDirectory(entity, Directory(destinationPath));
    }
  }
  return copiedFiles;
}

int _copyDirectory(Directory sourceDir, Directory destinationDir) {
  destinationDir.createSync(recursive: true);
  var copiedFiles = 0;
  for (final entity in sourceDir.listSync(followLinks: false)) {
    final name = _entityName(entity.path);
    if (name.isEmpty || name.startsWith('.')) {
      continue;
    }

    final destinationPath =
        '${destinationDir.path}${Platform.pathSeparator}$name';
    if (entity is File) {
      entity.copySync(destinationPath);
      copiedFiles++;
    } else if (entity is Directory) {
      copiedFiles += _copyDirectory(entity, Directory(destinationPath));
    }
  }
  return copiedFiles;
}

String _entityName(String path) {
  final normalized = path.replaceAll('\\', '/');
  final segments = normalized.split('/');
  return segments.isEmpty ? '' : segments.last;
}
