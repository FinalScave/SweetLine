---
name: syntax-highlighting-authoring
description: Use when adding or refining syntax highlighting rules, example files, routing behavior, or syntax-focused tests in any repository. Covers dialect split decisions, file routing strategy, example coverage standards, and validation gates for high-quality syntax definitions.
---

# Syntax Highlighting Authoring

Use this skill when the task involves creating or refining syntax rules, syntax routing, example files, or syntax-focused tests.

Read these references as needed:
- Read [language-family-decisions.md](references/language-family-decisions.md) before deciding whether to extend an existing syntax or create a new one.
- Read [routing-strategy.md](references/routing-strategy.md) before editing file matching or introducing a syntax with ambiguous extensions.
- Read [example-coverage-matrix.md](references/example-coverage-matrix.md) before writing or expanding any example file.
- Read the family template that matches the task before authoring token rules:
  - [patterns-dsl.md](references/patterns-dsl.md)
  - [patterns-jsx-tsx.md](references/patterns-jsx-tsx.md)
  - [patterns-assembly.md](references/patterns-assembly.md)
  - [patterns-config-schema.md](references/patterns-config-schema.md)
  - [patterns-script-languages.md](references/patterns-script-languages.md)
- Read [common-failures.md](references/common-failures.md) before final review.
- Read [validation-checklist.md](references/validation-checklist.md) before closing the task.

## Required workflow

1. Inspect at least two nearby syntaxes or the closest existing family before writing new rules.
2. Decide the correct scope first:
   - extend an existing syntax
   - create a new syntax
   - split a dialect or DSL from a base language
3. Decide routing before token work. Do not assume extension-only routing is enough.
4. Author the syntax with reusable fragments for comments, strings, escapes, URLs, and nested constructs whenever reuse is possible.
5. Keep tokenization fine-grained. Split meaningful token classes instead of relying on one broad fallback pattern.
6. Add or expand a realistic example file.
7. Add routing coverage, syntax compilation coverage, and at least one focused highlight assertion.
8. Run validation before finishing.
9. Review the common failure list and confirm that none of the known failure modes are present.

## Hard constraints

- Do not add a new syntax before checking whether the request is actually:
  - a dialect
  - a DSL
  - an embedded language
  - a file-routing problem
- Do not ship a syntax that only works for one happy-path snippet.
- Do not use a giant catch-all regex when the language has separable tokens such as directives, labels, properties, attributes, tags, or builtins.
- Do not leave repeated constructs duplicated across states when a fragment can express them once.
- Do not write filler examples.
- Do not leave example files below the project target range when the project has one.
- Do not skip routing checks when file extensions are ambiguous, overloaded, or multi-part.
- Do not declare a generic assembly syntax when the real need is multiple dialects.
- Do not close the task without exact-style assertions for at least one representative language-specific feature.
- Do not treat line-count compliance as sufficient example quality.
- Do not stop after syntax JSON compiles if analyzer routing, examples, or focused assertions are still missing.
- Do not assume a base language syntax is good enough for a DSL, component language, or schema language without comparing its required token classes against the family template.

## Example rule

When the repository does not define a different policy, use this default:
- Example files should target `120` to `150` lines.
- They should be realistic and dense with syntax coverage, not padded with repeated near-duplicates.

## Minimum deliverables

- Syntax definition or syntax refinement
- Example file or example expansion
- Routing update when needed
- Syntax compilation coverage
- Analyzer selection coverage
- Focused highlight assertions

## Definition of done

The task is not complete until every applicable item below is true:

- The syntax JSON is valid.
- The syntax compiles from file.
- Imported syntaxes are available in a valid compile order.
- Analyzer selection works for every intended routing path.
- The example file is within the repository target range.
- The example covers the hard branches of the grammar instead of only declarations and strings.
- At least one exact-style assertion proves a language-specific token category.
- Known failure modes from [common-failures.md](references/common-failures.md) have been checked and ruled out.
- Demo registration or asset routing is updated when the repository exposes syntax examples through a demo.

## Decision checkpoints

Before writing rules, explicitly answer these questions:

- Is this a new syntax, a dialect split, a DSL split, or a refinement of an existing syntax?
- Is extension-only routing enough?
- Which family template applies?
- Which token categories must remain distinct for this language to feel high quality?
- Which edge cases are likely to regress if the example and tests do not cover them?

## Output expectations

Summarize what was added in terms of:
- routing behavior
- token classes covered
- example coverage
- validation performed
