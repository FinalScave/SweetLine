---
name: sweetline-syntax-profile
description: Use with syntax-highlighting-authoring when working in the SweetLine repository. Adds repository-specific paths, style names, testing surfaces, demo registration points, and current routing caveats for SweetLine syntax work.
---

# SweetLine Syntax Profile

Use this profile together with `syntax-highlighting-authoring` when editing the SweetLine repository.

Read [repo-map.md](references/repo-map.md) before editing files.

## Repository-specific rules

- Syntax files live in `syntaxes/*.json`.
- Example files live in `tests/files/example.*`.
- New or heavily revised examples should target `120` to `150` lines.
- Syntax compilation coverage belongs in `tests/syntax_test.cpp`.
- Focused highlight assertions belong in `tests/highlight_test.cpp`.
- If the syntax affects indentation-sensitive behavior, inspect `tests/indent_test.cpp`.

## SweetLine style names

Prefer the existing shared style vocabulary:
- `keyword`
- `string`
- `number`
- `comment`
- `class`
- `method`
- `variable`
- `punctuation`
- `annotation`
- `builtin`
- `preprocessor`
- `macro`
- `property`
- `lifetime`
- `selector`
- `url`

## SweetLine-specific routing caveat

Do not assume the current branch supports filename-aware routing.

Before relying on:
- exact file names
- file name suffixes
- file name patterns

first verify that the current branch supports them. If it does not, treat routing support as part of the task.

This matters for syntaxes such as:
- `gradle` and `gradle-kts`
- assembly dialects
- any syntax that depends on multi-part names

## SweetLine-specific conflict checks

Before splitting or adding a syntax, check for collisions with existing file types:
- Objective-C and MATLAB can both involve `.m`
- shell-style config files may overlap with existing config syntaxes
- multi-part file names may need both core routing changes and demo-side routing changes

Do not change a shared extension mapping without deciding:
- whether core routing must be upgraded
- whether tests need file-name-based analyzer coverage
- whether demos still choose syntax by extension only

## SweetLine delivery bar

For every new syntax or major syntax split:
- add the syntax file
- add or expand the example file
- compile the syntax in `tests/syntax_test.cpp`
- cover analyzer selection in `tests/syntax_test.cpp`
- add at least one exact-style highlight test
- update demo registration when the syntax should appear in shipped demos

## Demo note

SweetLine demos maintain their own syntax maps. If a new syntax should be selectable in demos, update the demo-side routing and asset lists in the current branch.
