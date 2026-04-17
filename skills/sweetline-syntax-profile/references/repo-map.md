# SweetLine Repo Map

Use this file when wiring a syntax change through the repository.

## Syntax and examples

- `syntaxes/*.json`
- `tests/files/example.*`

## Syntax-focused tests

- `tests/syntax_test.cpp`
  - compile built-in syntaxes
  - analyzer creation by extension
  - fragment and import behavior tests
- `tests/highlight_test.cpp`
  - focused style assertions
  - representative language-family regression tests
- `tests/indent_test.cpp`
  - indentation and scope-guide behavior

## Common style IDs in tests

- `keyword` `1`
- `string` `2`
- `number` `3`
- `comment` `4`
- `class` `5`
- `method` `6`
- `variable` `7`
- `punctuation` `8`
- `annotation` `9`
- `builtin` `10`
- `preprocessor` `11`
- `macro` `12`
- `property` `13`
- `lifetime` `14`
- `selector` `15`
- `url` `16`

## Demo surfaces commonly touched by syntax additions

- Android:
  - `platform/Android/app/src/main/java/com/qiplat/sweetline/demo/MainActivity.java`
  - `platform/Android/app/src/main/java/com/qiplat/sweetline/demo/MarkwonActivity.java`
- Java22:
  - `platform/Java22/demo/src/main/java/com/qiplat/sweetline/demo/Main.java`
- Flutter:
  - `platform/Flutter/demo/lib/main.dart`
  - `platform/Flutter/demo/tool/sync_demo_assets.dart`
  - `platform/Flutter/demo/lib/generated/demo_assets.g.dart`
- OHOS:
  - `platform/OHOS/demo/src/main/ets/pages/Index.ets`
- Apple:
  - `platform/Apple/Examples-MacOS/Sources/SweetLineDemoSupport/DemoSampleSupport.swift`
- Emscripten:
  - `platform/Emscripten/demo.html`

## Validation commands commonly useful in this repo

- `jq empty syntaxes/<name>.json`
- `ninja -C build tests/CMakeFiles/test.dir/syntax_test.cpp.o tests/CMakeFiles/test.dir/highlight_test.cpp.o`
- run focused test filters when a temporary test binary is already available

## Practical reminders

- Reuse fragments for comments, strings, escapes, and URLs.
- Keep example files realistic and within `120` to `150` lines.
- If a syntax reuses an ambiguous extension, verify whether core routing and demo routing both need changes.
- Watch for extension collisions such as Objective-C versus MATLAB on `.m`.
