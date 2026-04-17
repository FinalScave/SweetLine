# SweetLine Validation Checklist

Do not finish syntax work until you have checked the items that apply.

## Syntax definition checks

- JSON syntax is valid.
- The syntax compiles from file.
- Imported syntaxes are available before the dependent syntax is compiled.
- New fragments and states are actually reachable.
- Group numbering intent is explicit: `styles` and `subStates` use token-local capture indices, helper captures are preserved when needed to stabilize numbering, and non-capturing groups are used everywhere else.
- `include` / `includes`, `importSyntax`, and `onLineEndState` directive objects are written as standalone objects rather than mixed with token fields.
- For inline-style syntaxes, every referenced inline style name exists in top-level `styles[]`.
- If the syntax uses `scopeRules`, `start` / `end` are strings, `branches` is a string array when present, and delimiter or branch tokens are not accidentally hidden behind scope-skip styles such as names containing `string`, `comment`, or `char`.

## Routing checks

- Analyzer selection works for every intended exact file name, file suffix, or basename-level file-name pattern.
- Ambiguous file names, suffixes, or patterns do not route to the wrong syntax.
- Exact-name duplicates and duplicate syntax names have not been introduced.
- File-name patterns were validated as full matches rather than substring matches.
- Analyzer-selection tests cover the intended core routing path with the real routed file names, and sample files or demo assets remain aligned with the syntax family they are meant to exercise.

## Example checks

- If an example is added or modified, its final line count is within the SweetLine target range.
- If an example is added or modified, it covers the hard parts of the grammar.
- If an example is added or modified, representative embedded inline tokens are present when the syntax intentionally highlights them.

## Highlight checks

- At least one focused test asserts exact style IDs for language-specific tokens.
- The assertion covers something more specific than plain keywords or strings.
- Representative tokens map to the correct shared style classes from `style-vocabulary-policy.md`.
- Grouped rules have been checked so `style`, `styles`, and `subStates` align with the intended full-match or subgroup tokenization.
- Meaningful tokens do not fall back to default style because of grouping, ordering, or fallback mistakes.

## Regression checks

- Syntax-related tests compile.
- The relevant test subset passes when the environment permits it.
- Macro-gated imports are verified with the intended macro definitions when the syntax uses `#ifdef`.
- Demo asset sync, generated manifest, or built-in syntax/example list is updated when the project exposes examples in a demo.
