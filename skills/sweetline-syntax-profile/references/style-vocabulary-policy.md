# Style Vocabulary Policy

Use this file when assigning style names in SweetLine syntax rules.

## Purpose

SweetLine syntax work should reuse the existing shared style vocabulary by default.

Adding a new syntax does not imply adding a new style name. Prefer the closest semantic class from the existing style set unless the token meaning clearly falls outside it.

## Default rule

- Reuse the existing `16` style names first.
- Choose the closest semantic class, not the most visually attractive color bucket.
- Do not add a new style name just to make one syntax feel more granular.
- If a token can be classified reasonably with an existing style, it must use that style.

## Existing style vocabulary

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

## Core distinctions

### `keyword`

Use `keyword` for:
- reserved words
- control words
- declaration heads
- structural DSL block heads
- reserved literals such as boolean or null-like values when they behave like language keywords

Do not use `keyword` for ordinary library APIs, field names, or user-defined symbols.

### `builtin`

Use `builtin` for:
- intrinsic types
- standard built-in constants
- standard built-in functions
- host-environment builtins that are not ordinary user-defined symbols

Do not use `builtin` for every framework API or every well-known library symbol.

### `property`

Use `property` for:
- object member names when the syntax models them explicitly
- configuration keys
- schema field names
- attribute names
- option names
- named assignments where the token is a key rather than a callable symbol

Do not use `property` as a generic bucket for every identifier that appears after punctuation.

### `selector`

Use `selector` for:
- CSS-like selectors
- glob-like selectors
- route or pattern selectors
- Make targets and other names that primarily select a target rather than define a callable or a type

Do not use `selector` as a generic bucket for special-looking identifiers.

### `annotation`

Use `annotation` for:
- decorators
- attributes
- pragma-like markers
- sigil-prefixed metadata tokens

Do not use `annotation` for ordinary sigils unless they actually express metadata or declarative modifiers.

## Additional guidance for other shared styles

- Use `class` for type-like declarations or references that the syntax clearly models as classes, interfaces, traits, records, messages, or similar named types.
- Use `method` for callable symbols when the syntax clearly models them as functions, methods, commands, or named helpers.
- Use `variable` for named runtime values, placeholders, interpolation variables, or binding names.
- Use `preprocessor` for preprocessor directives or compiler-control directives.
- Use `macro` for macro names or macro-like invocations that are distinct from ordinary callables.
- Use `lifetime` only for lifetime-like markers or ownership annotations that the syntax explicitly models that way.
- Use `punctuation` only for structural punctuation, delimiters, operators, and separators.
- Use `url` only when the syntax intentionally recognizes URL-like inline tokens as a dedicated class.

## Common misclassification rules

- Do not use `builtin` when the token is really a reserved control word or declaration head.
- Do not use `keyword` when the token is really a typed builtin or intrinsic symbol.
- Do not use `property` when the token is actually a selector, annotation, type, or callable.
- Do not use `class`, `method`, or `variable` when the syntax is primarily modeling key-value structure rather than executable symbols.
- Do not create a new style name just because one syntax has a nuanced domain concept.

## When a new style name is allowed

Only allow a new style name when all of these are true:
- no existing style expresses the token semantics well enough
- the new style is expected to be reusable across multiple syntaxes
- tests, demos, and style maps will all be updated together
- the change summary can explain clearly why the token cannot be mapped to an existing style

If any of those conditions are not met, reuse an existing style.

## Required validation

- Check representative tokens against this policy in focused highlight tests.
- If a style choice is non-obvious, explain the mapping in the change summary.
- If a new style name seems necessary, stop and get explicit approval before adding it.
