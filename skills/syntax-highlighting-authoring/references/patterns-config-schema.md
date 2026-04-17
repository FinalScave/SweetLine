# Patterns for Config and Schema Languages

Use this template for formats such as schema definitions, typed config languages, and structured data dialects.

## Distinct token classes to prefer

- declarations or block heads
- field or key names
- scalar type names
- builtins and reserved literals
- directives or options
- enum members
- strings
- numbers
- comments
- punctuation
- URLs

## Fragment guidance

Reuse fragments for:
- line comments
- block comments
- strings
- URL matching in string and comment content

## Example requirements

A strong config or schema example should include:
- nested structures
- repeated or array-like fields
- enum-like members
- options or directives
- scalar and structured values
- multiline descriptions or heredoc-like content when supported
- realistic URLs, versions, identifiers, and paths

## Common mistakes

- Treating keys, field names, and type names as the same token class
- Ignoring options, directives, or annotations
- Only demonstrating flat objects with no nesting
