# SweetLine Scope Decision Guide

Use this file before deciding whether a task should refine an existing syntax, add a new syntax, split a dialect, or only change routing.

## Refine an existing syntax when

- the grammar is fundamentally the same
- the task adds missing token classes or missing edge cases
- the task improves existing strings, comments, escapes, grouping, or embedded-token handling
- the file-routing behavior does not need to diverge from the existing syntax

## Create a new syntax when

- the target introduces distinct structural tokens or block heads
- the target embeds multiple sublanguages that need dedicated handling
- the target is a DSL whose key token classes do not fit the base language well
- reusing the base syntax would collapse meaningful tokens into overly generic classes

## Split a dialect or DSL from a base language when

- operand markers, directives, registers, selectors, or declarations differ materially
- the same broad language family needs different token classes or different state structure
- reusing one syntax would force broad fallbacks or confusing style mappings

## Treat it as a routing problem when

- the grammar is unchanged but file selection is ambiguous
- the distinction is driven by exact file names, multi-part suffixes, or file-name patterns
- the existing syntax is correct once the right analyzer is selected

## Quick check

Before creating a new syntax, answer these questions:

- Does the target really need different grammar handling, or only better routing?
- Would refining the existing syntax preserve meaningful token distinctions?
- Would a split reduce ambiguity, or only duplicate the same grammar under a different name?
- Can the current `fileName`, `fileSuffix`, `fileNamePattern`, `variables`, `fragments`, `states`, `style/styles`, `subStates`, `importSyntax`, `include/includes`, `onLineEndState`, and `scopeRules` model express the need without touching core code?
