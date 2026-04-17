# Patterns for DSLs

Use this template for build DSLs, infrastructure DSLs, framework config DSLs, and similar structured languages.

## Distinct token classes to prefer

- block heads
- property keys
- helper methods
- dependency or plugin configuration names
- classes or types when the DSL references typed objects
- builtins or reserved literals
- strings
- numbers
- comments
- punctuation
- URLs

## Fragment guidance

Prefer reusable fragments for:
- line comments
- block comments
- single and double strings
- escape sequences
- URLs in comments and strings

## State guidance

Most DSLs work best with:
- default block parsing
- string content states when interpolation or URL reuse is needed
- comment content states when URL reuse is needed
- nested expression or generic states only when the DSL embeds a host language

## Example requirements

A strong DSL example should include:
- multiple top-level blocks
- nested blocks
- property assignment styles
- list and map forms
- helper calls and factory calls
- interpolation or provider access when supported
- realistic versions, paths, URLs, identifiers, and flags

## Common mistakes

- Treating DSL block heads as ordinary method names
- Leaving property keys indistinguishable from values
- Missing configuration-specific names such as dependency scopes or plugin aliases
- Testing only one block and one string
