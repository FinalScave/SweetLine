# Language Family Decisions

Use this file before choosing between extending an existing syntax and creating a new one.

## Create a new syntax when

- The target is a DSL with recognizable block heads or directives.
- The target is a language superset with new structural tokens.
- The target is an embedded format with distinct tag or interpolation rules.
- The target is a dialect with conflicting token grammar.
- The target needs routing by file name instead of plain extension.

Typical examples:
- `gradle` versus plain `groovy`
- `gradle-kts` versus plain `kotlin`
- `jsx` versus plain `javascript`
- `tsx` versus plain `typescript`
- `asm-nasm`, `asm-gas`, and `asm-aarch64` instead of one generic `asm`

## Extend an existing syntax when

- The file type is only a small lexical variation.
- The new request adds missing token classes without changing structure.
- The extension is new but the language grammar is unchanged.

Typical examples:
- adding more builtins
- adding URL reuse inside comments and strings
- refining property or selector detection
- supporting another extension for the same grammar

## Prefer family splits for these categories

### DSLs

Split when the DSL introduces:
- named block heads
- configuration-specific properties
- helper calls that deserve dedicated token classes
- routing by exact file name

### Embedded markup languages

Split when the format combines:
- markup
- embedded script
- embedded style
- interpolation or directive syntax

### Assembly

Split by dialect or architecture whenever operand markers, register syntax, directives, or instruction families differ materially.

### Template and component files

Split when the syntax needs to recognize:
- tags
- component tags
- attributes
- directive attributes
- embedded expressions
- comments and strings inside multiple sublanguages
