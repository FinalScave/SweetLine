# Patterns for JSX and TSX

Use this template for component syntaxes that blend JavaScript or TypeScript with markup-like tag structures.

## Distinct token classes to prefer

- imports and exports through the host language
- native tags
- component tags
- fragment syntax
- attribute names
- event handler attributes
- spread props
- inline expressions
- strings
- comments
- URLs
- punctuation

For TSX also keep these distinct when possible:
- interfaces and type aliases
- generic type parameters
- typed handlers
- builtin utility types

## Fragment guidance

Reuse fragments for:
- strings and escapes from the host language
- URL matching in strings and comments
- JSX comment bodies
- attribute value strings

## State guidance

Common useful states:
- default host language state
- tag-open or tag-header state
- attribute parsing state
- embedded expression state for `{...}`
- multiline comment state when JSX comment forms need it

## Example requirements

A strong JSX or TSX example should include:
- imports and exports
- function components
- native tags and component tags
- attributes and event handlers
- spread props
- inline expressions
- conditional rendering
- list rendering
- comments in both host and JSX contexts
- strings and template strings with URLs

## Common mistakes

- Treating component tags and native tags the same when the syntax wants them distinct
- Ignoring attributes and only highlighting the tag name
- Missing JSX comment forms
- Forgetting that TSX needs both host language typing and tag structure
