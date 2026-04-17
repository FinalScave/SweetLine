# Patterns for Assembly

Use this template for any assembly syntax. Prefer dialect-specific syntaxes over a generic catch-all.

## Split guidance

Create separate syntaxes when any of these differ materially:
- register prefixing or naming
- immediate operand markers
- memory operand notation
- directive vocabulary
- relocation markers
- architecture-specific instruction families

## Distinct token classes to prefer

- directives
- section names
- labels
- local labels
- opcodes
- registers
- immediates
- numbers in multiple bases
- memory operands
- relocations or modifiers
- macros or pseudo-ops
- strings
- comments
- URLs
- punctuation

## Fragment guidance

Reuse fragments for:
- line comments
- block comments when supported
- strings
- numbers
- URLs in strings and comments

## Example requirements

A strong assembly example should include:
- multiple sections
- exported and external symbols
- labels and local labels
- several opcode families
- register use across operand forms
- immediate values and memory operands
- macro definitions or pseudo-ops
- strings and comments with URLs
- relocations or modifiers if the dialect supports them

## Common mistakes

- Shipping one generic `asm` syntax for multiple incompatible dialects
- Highlighting registers and opcodes but ignoring directives and relocations
- Omitting local labels and macros from examples
- Using only one numeric base
