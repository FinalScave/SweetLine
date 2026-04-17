# SweetLine Validation Checklist

Do not finish syntax work until you have checked the items that apply.

## Syntax definition checks

- JSON syntax is valid.
- The syntax compiles from file.
- Imported syntaxes are available before the dependent syntax is compiled.
- New fragments and states are actually reachable.
- Grouping intent is explicit: structural regex groups are non-capturing, and any capturing group is intentionally consumed by `styles` or `subStates`.

## Routing checks

- Analyzer selection works for every intended exact file name, file suffix, or file-name pattern.
- Ambiguous file names, suffixes, or patterns do not route to the wrong syntax.
- Sample-file routing and demo-side routing remain aligned when both exist.

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
- Demo registration or asset sync is updated when the project exposes examples in a demo.
