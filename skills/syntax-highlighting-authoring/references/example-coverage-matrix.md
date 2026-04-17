# Example Coverage Matrix

Every example file should exercise the syntax, not just demonstrate it.

## Default size target

- Target `120` to `150` lines unless the repository defines a different range.

## Universal coverage checklist

Every example should cover as many of these as the language supports:
- declarations or definitions
- imports, includes, packages, or modules
- single-line comments
- multiline comments or block comments when supported
- strings
- escapes or interpolation when supported
- URLs in comments and strings when the syntax highlights URLs
- numeric literal variants
- nested blocks or state transitions
- punctuation-heavy constructs
- builtins or library forms
- edge constructs that are easy to regress

## Family-specific additions

### DSLs and config languages

Include:
- top-level blocks
- nested blocks
- property assignments
- list and map forms
- environment-specific or flavor-specific branches
- realistic URLs, paths, versions, and identifiers

### JSX, TSX, and component syntaxes

Include:
- imports and exports
- native tags and component tags
- attributes and event handlers
- spread props
- inline expressions
- conditional rendering
- list rendering
- comments in both host and embedded contexts
- strings and template strings with URLs

### Assembly

Include:
- sections
- directives
- labels
- local labels when supported
- opcodes
- registers
- memory operands
- immediate values
- relocations or modifiers when supported
- macros or pseudo-ops
- strings and comments

### Data and schema languages

Include:
- nested objects or messages
- enum-like constructs
- option or directive forms
- arrays or repeated fields
- scalar and structured values
- multiline descriptions when supported

## What to avoid

- repeated blocks with only identifier changes
- empty spacer sections used only to hit a line count
- toy examples that skip the hard parts of the grammar
