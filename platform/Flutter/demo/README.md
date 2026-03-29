# demo

Flutter demo for SweetLine.

## Prepare demo assets

The demo reads syntax JSON files and example source files from
`demo/assets/...`, but those assets are generated from the repository root
sources. Sync them before running the demo after changing `syntaxes/` or
`tests/files/`:

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter
dart run tool/sync_demo_assets.dart
```

`demo/assets/examples` and `demo/assets/syntaxes` are generated outputs and
should not be maintained by hand.

## Run

Before running the demo, also sync the package-native binaries:

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter/sweetline
dart tool/sync_native_binaries.dart
```

Then run the demo:

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter/demo
flutter run
```
