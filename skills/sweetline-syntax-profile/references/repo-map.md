# SweetLine Repo Map

Use this file when wiring a syntax change through the repository.

## Syntax and examples

- `syntaxes/*.json`
- `tests/files/example.*` for the main example set
- occasional syntax-specific self-hosting assets such as `syntaxes/json-sweetline.json`
- the example set also includes alias-style routed names such as `example.aarch64`, `example.myu`, and `example.t`

## Syntax-focused tests

- `tests/syntax_test.cpp`
  - compile built-in syntaxes
  - analyzer creation by file name
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

Most native demos now precompile common syntaxes and rely on core file-name routing, so syntax additions usually do not require demo-side routing edits.

The shipped demo surfaces still commonly touched are:
- Flutter:
  - `platform/Flutter/demo/tool/sync_demo_assets.dart`
  - `platform/Flutter/demo/lib/generated/demo_assets.g.dart`
  - `platform/Flutter/demo/lib/main.dart`
- Emscripten:
  - `platform/Emscripten/demo.html`

## Validation commands commonly useful in this repo

- `jq empty syntaxes/<name>.json`
- `ninja -C build tests/CMakeFiles/test.dir/syntax_test.cpp.o tests/CMakeFiles/test.dir/highlight_test.cpp.o`
- run focused test filters when a temporary test binary is already available

## Practical reminders

- Reuse fragments for comments, strings, escapes, and embedded inline tokens.
- Keep new or modified example files realistic and within `120` to `250` lines.
- Route test names should reflect the real routed basename, especially for exact-name routes and alias-style sample names.
- `CMakeLists.txt`, `Dockerfile`, `Containerfile`, `Makefile`, `makefile`, `GNUmakefile`, `BSDmakefile`, and `.gitignore` are exact-name routing cases worth checking explicitly.
- `generated.route` and `templated.route` are full-match pattern examples; `generated.route.bak` and `prefix-generated.route` should not route.
- If a syntax reuses an ambiguous suffix, verify core routing first, then update only the demo asset exposure layers that still enumerate shipped syntaxes or examples.
- Do not ship route-colliding variants together in a common-syntax bundle.
- Watch for suffix collisions such as Objective-C versus MATLAB on `.m`.
