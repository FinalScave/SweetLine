# Validation Checklist

Do not finish syntax work until you have checked the items that apply.

## Syntax definition checks

- JSON syntax is valid.
- The syntax compiles from file.
- Imported syntaxes are available before the dependent syntax is compiled.
- New fragments and states are actually reachable.

## Routing checks

- Analyzer selection works for every intended file extension.
- Analyzer selection also works for exact file names, suffixes, or patterns when used.
- Ambiguous extensions do not route to the wrong syntax.

## Example checks

- The example line count is within the project target range.
- The example covers the hard parts of the grammar.
- URLs are present in strings or comments when URL highlighting is supported.

## Highlight checks

- At least one focused test asserts exact style IDs for language-specific tokens.
- The assertion covers something more specific than plain keywords or strings.

## Regression checks

- Syntax-related tests compile.
- The relevant test subset passes when the environment permits it.
- Demo registration or asset sync is updated when the project exposes examples in a demo.
