# Patterns for Script Languages

Use this template for scripting languages with rich string, regex, or sigil-heavy behavior.

## Distinct token classes to prefer

- declarations
- builtins
- variables
- method or function names
- package or module names
- string forms
- comments
- regex forms
- numeric literal variants
- punctuation
- URLs

## Fragment guidance

Reuse fragments for:
- line comments
- block comments or documentation comments
- multiple string forms
- escapes and interpolation
- URL matching in strings and comments

## Example requirements

A strong script-language example should include:
- declarations and imports
- common builtins
- multiple string forms
- comments and documentation comments
- regex or pattern forms when supported
- collections and nested access
- control flow
- realistic script tasks such as parsing, reporting, or automation

## Common mistakes

- Only handling one string form when the language has several
- Treating regex as plain strings
- Ignoring sigil or namespace forms
- Missing documentation comment formats
