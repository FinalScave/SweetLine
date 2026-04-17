---
name: sweetline-syntax-profile
description: Use when adding or refining SweetLine syntax rules, routing behavior, example files, style mapping, or syntax-focused tests. Covers repository-specific paths, style vocabulary, validation, failure modes, and the SweetLine file-name routing model.
---

# SweetLine Syntax Profile

Use this skill when editing syntax rules in the SweetLine repository.

Read [repo-map.md](references/repo-map.md) before editing files.
Read [scope-decision-guide.md](references/scope-decision-guide.md) before deciding whether the task is a refinement, a new syntax, a dialect split, or a routing problem.
Read [style-vocabulary-policy.md](references/style-vocabulary-policy.md) before assigning style names or proposing a new style name.
Read [validation-checklist.md](references/validation-checklist.md) before closing the task.
Read [common-failures.md](references/common-failures.md) before final review.

## Repository-specific rules

- Syntax files live in `syntaxes/*.json`.
- Example files live in `tests/files/example.*`.
- New examples and modified examples should end within `120` to `250` lines.
- Syntax compilation coverage belongs in `tests/syntax_test.cpp`.
- Focused highlight assertions belong in `tests/highlight_test.cpp`.
- If the syntax affects indentation-sensitive behavior, inspect `tests/indent_test.cpp`.

## Required workflow

1. Inspect at least two nearby syntaxes or the closest existing family before writing new rules.
2. Decide the correct scope first. Use [scope-decision-guide.md](references/scope-decision-guide.md) to determine whether the task is:
   - extend an existing syntax
   - create a new syntax
   - split a dialect or DSL from a base language
   - solve a file-routing problem
3. Decide file-name-based routing before token work. Do not assume suffix-only routing is enough.
4. Reuse existing fragments and states when they already express the needed comment, string, escape, or embedded-token behavior.
5. Keep tokenization fine-grained. Split meaningful token classes instead of relying on one broad fallback pattern.
6. Add or expand a realistic example file when the task introduces a new syntax, a major syntax split, or leaves the current example clearly under-covered. Keep any new or modified example within the SweetLine target range.
7. Add or update syntax compilation coverage, analyzer-selection coverage, and at least one focused highlight assertion when the task changes syntax behavior or analyzer routing.
8. Run validation before finishing.
9. Review the common failure list and confirm that none of the known failure modes are present.

## SweetLine core boundary

For syntax additions and normal syntax refinements, do not modify these areas by default:
- `src/core`
- `src/include`
- public APIs
- bindings
- demo routing infrastructure

Exhaust the existing syntax JSON model first:
- `fileName` / `fileNames`
- `fileSuffix` / `fileSuffixes`
- `fileNamePattern` / `fileNamePatterns`
- `variables`
- fragments
- states
- `style` / `styles`
- `subStates`
- `importSyntax`
- `include` / `includes`
- `onLineEndState`
- `scopeRules`

Delivery work may still be required after a syntax-only solution:
- syntax tests
- highlight tests
- demo registration

Only consider core or API changes when the requirement cannot be expressed with the current SweetLine syntax model and routing behavior.

If a core or binding change seems necessary, stop and get explicit user confirmation before editing any of those files.

## Hard constraints

- Do not add a new syntax before checking whether the request is actually a refinement, a dialect split, a DSL split, or a file-routing problem.
- Do not ship a syntax that only works for one happy-path snippet.
- Do not use a giant catch-all regex when the language has separable tokens such as directives, labels, properties, attributes, tags, or builtins.
- Prefer non-capturing groups by default. Introduce capturing groups only when they are intentionally consumed by `styles` or `subStates`.
- Do not leave structural regex groups as capturing groups when the whole match should use one token class.
- Do not leave repeated constructs duplicated across states when a fragment can express them once.
- Do not write filler examples.
- Do not leave new or modified example files outside the SweetLine target range.
- Do not skip routing checks when file names, suffixes, or patterns are ambiguous, overloaded, or multi-part.
- Do not expand a syntax task into engine, parser, routing-infrastructure, or public-API changes unless the current SweetLine syntax model cannot express the requirement.
- Do not close the task without exact-style assertions for at least one representative language-specific feature.
- Do not treat line-count compliance as sufficient example quality.
- Do not stop after syntax JSON compiles if analyzer routing, examples, or focused assertions are still missing.
- Do not treat style assertions as complete unless representative tokens are mapped to the correct shared style class from [style-vocabulary-policy.md](references/style-vocabulary-policy.md).

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

## SweetLine-specific routing model

SweetLine routing is file-name based.

Prefer these fields in syntax JSON:
- `fileName` / `fileNames`
- `fileSuffix` / `fileSuffixes`
- `fileNamePattern` / `fileNamePatterns`

Resolve routes in this order:
- exact file names
- file suffixes
- file name patterns

This matters for syntaxes such as:
- `gradle` and `gradle-kts`
- assembly dialects
- any syntax that depends on multi-part names or conventional file names

## SweetLine delivery bar

For every new syntax or major syntax split:
- add the syntax file
- add or expand the example file
- compile the syntax in `tests/syntax_test.cpp`
- cover analyzer selection in `tests/syntax_test.cpp`
- add at least one exact-style highlight test
- update demo registration when the syntax should appear in shipped demos

## Definition of done

The task is not complete until every applicable item below is true:

- The syntax JSON is valid.
- The syntax compiles from file.
- Imported syntaxes are available in a valid compile order.
- Analyzer selection works for every intended routing path.
- New or modified example files are within the SweetLine target range.
- New or modified example files cover hard branches of the grammar instead of only declarations and strings.
- At least one exact-style assertion proves a changed or language-specific token category.
- Representative style assignments follow [style-vocabulary-policy.md](references/style-vocabulary-policy.md).
- Known failure modes from [common-failures.md](references/common-failures.md) have been checked and ruled out.
- Demo registration or asset routing is updated when the syntax appears in shipped demos.

## Demo note

SweetLine demos maintain their own syntax maps. If a new syntax should be selectable in demos, update the demo-side routing and asset lists in the current branch.
