import 'dart:io';
import 'package:ffigen/ffigen.dart';

const _assetId = 'package:sweetline/lib/sweetline.dart';

void main() {
  final packageRoot = Platform.script.resolve('../');
  final outputFile = File(
    packageRoot.resolve('lib/sweetline_bindings_generated.dart').toFilePath(),
  );

  FfiGenerator(
    output: Output(dartFile: outputFile.uri),
    functions: Functions(
      include: (Declaration declaration) =>
          declaration.originalName.startsWith('sl_'),
    ),
    structs: Structs(
      include: (Declaration declaration) =>
          declaration.originalName.startsWith('sl_'),
    ),
    typedefs: Typedefs(
      include: (Declaration declaration) =>
          declaration.originalName.startsWith('sl_'),
    ),
    headers: Headers(
      entryPoints: [packageRoot.resolve('src/c_sweetline.h')],
      compilerOptions: [
        '-I${packageRoot.resolve('src').toFilePath()}',
        '-DSWEETLINE_EXPORTS',
      ],
    ),
  ).generate();

  _patchGeneratedBindings(outputFile);
}

void _patchGeneratedBindings(File file) {
  var content = file.readAsStringSync();
  final assetIdDeclaration = "const _sweetlineAssetId = '$_assetId';";

  if (!content.contains(assetIdDeclaration)) {
    content = content.replaceFirst(
      "import 'dart:ffi' as ffi;\n",
      "import 'dart:ffi' as ffi;\n\n$assetIdDeclaration\n",
    );
  }

  content = content.replaceAllMapped(
    RegExp(r'@ffi\.Native<([\s\S]*?)>\(([\s\S]*?)\)', dotAll: true),
    (match) {
      final generic = match.group(1)!;
      final args = match.group(2)!;
      if (args.contains('assetId:')) {
        return match.group(0)!;
      }
      if (args.trim().isEmpty) {
        return '@ffi.Native<$generic>(assetId: _sweetlineAssetId)';
      }
      if (args.startsWith('\r\n') || args.startsWith('\n')) {
        return '@ffi.Native<$generic>(assetId: _sweetlineAssetId,$args)';
      }
      return '@ffi.Native<$generic>(assetId: _sweetlineAssetId, $args)';
    },
  );

  file.writeAsStringSync(content);
}
