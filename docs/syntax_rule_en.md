# SweetLine Syntax Rule Configuration Guide

SweetLine uses JSON to define syntax highlighting rules. Each JSON file describes the highlighting rules for a programming language. The engine compiles these rules at runtime for text analysis.

## Table of Contents

- [Basic Structure](#basic-structure)
- [Top-Level Fields](#top-level-fields)
- [variables - Variable Definitions](#variables---variable-definitions)
- [states - State Machine Definition](#states---state-machine-definition)
- [Pattern Matching Rules](#pattern-matching-rules)
- [Style System](#style-system)
- [State Transitions](#state-transitions)
- [SubStates](#substates)
- [onLineEndState - Line End State](#onlineendstate---line-end-state)
- [blockPairs - Code Block Pairs](#blockpairs---code-block-pairs)
- [Inline Style Mode](#inline-style-mode)
- [Complete Example](#complete-example)
- [Best Practices](#best-practices)

---

## Basic Structure

A complete syntax rule JSON file has the following structure:

```json
{
  "name": "languageName",
  "fileExtensions": [".ext1", ".ext2"],
  "variables": { ... },
  "styles": [ ... ],
  "states": {
    "default": [ ... ],
    "stateName": [ ... ]
  },
  "blockPairs": [ ... ]
}
```

---

## Top-Level Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | string | Yes | Syntax rule name, used for `getSyntaxRuleByName()` lookup |
| `fileExtensions` | string[] | Yes | Supported file extensions, used for `getSyntaxRuleByExtension()` matching |
| `variables` | object | No | Reusable regex pattern variable definitions |
| `styles` | array | No | Inline style definitions (only for `inline_style` mode) |
| `states` | object | Yes | State machine definitions containing all states and their matching rules |
| `blockPairs` | array | No | Code block pair definitions (for folding/indent guides) |

---

## variables - Variable Definitions

`variables` defines reusable regex fragments that can be referenced in `pattern` using `${variableName}`. Variables can reference other variables (nested expansion is supported).

```json
{
  "variables": {
    "identifier": "[a-zA-Z_]\\w*",
    "whiteSpace": "[ \\t\\f]",
    "any": "[\\S\\s]",
    "primitiveType": "void|boolean|byte|char|short|int|long|float|double"
  }
}
```

**Usage in patterns:**

```json
{
  "pattern": "\\b(${primitiveType})\\b${whiteSpace}+(${identifier})",
  "styles": [1, "keyword", 2, "variable"]
}
```

After expansion, this is equivalent to:
```
\b(void|boolean|byte|char|short|int|long|float|double)\b[ \t\f]+([a-zA-Z_]\w*)
```

**Notes:**
- Variable names are case-sensitive
- Variables support nested references (e.g., `"identifier": "${identifierStart}${identifierPart}"`)
- SweetLine uses Oniguruma regex syntax, supporting `\p{Han}` (Unicode properties), lookaheads/lookbehinds, and other advanced features
- Backslashes must be double-escaped in JSON: regex `\b` is written as `\\b` in JSON

---

## states - State Machine Definition

`states` is the core of syntax rules, defining a Finite State Machine (FSM). Each state contains an ordered list of matching rules. The engine tries to match them in order and uses the first successful match.

### The `default` State is Required

`default` is the initial state — all text parsing starts from this state. Other states are entered via the `state` field.

```json
{
  "states": {
    "default": [
      { "pattern": "...", "style": "...", "state": "stringState" }
    ],
    "stringState": [
      { "pattern": "...", "style": "...", "state": "default" }
    ]
  }
}
```

### Rules Within a State

Each state is an array `[]` of **matching rule objects**. The engine tries to match each rule at the current position in array order. On a successful match, it applies the style and advances past the matched text.

---

## Pattern Matching Rules

Each matching rule object contains the following fields:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `pattern` | string | Yes* | Oniguruma regex, supports `${variableName}` substitution |
| `style` | string | No | Style name for the entire match (mutually exclusive with `styles`) |
| `styles` | array | No | Capture group style mapping (mutually exclusive with `style`) |
| `state` | string | No | Target state to transition to after a successful match |
| `subStates` | array | No | Delegate specific capture group content to a sub-state for processing |
| `onLineEndState` | string | No* | Auto-transition state at end of line (special rule, no `pattern`) |

> `*` Note: Regular matching rules must have `pattern`; line-end state rules only have `onLineEndState` without `pattern`.

### Basic Pattern Example

```json
{
  "pattern": "\\b(if|else|while|for|return)\\b",
  "styles": [1, "keyword"]
}
```

### Regex Reference

SweetLine uses [Oniguruma](https://github.com/kkos/oniguruma) as its regex engine, supporting:

- Standard regex: `\b`, `\w`, `\d`, `\s`, `.`, `*`, `+`, `?`, `|`, `()`, `[]`, etc.
- Unicode properties: `\p{Han}` (CJK characters), `\p{L}` (letters), etc.
- Lookaheads/lookbehinds: `(?=...)` lookahead, `(?<=...)` lookbehind, `(?!...)` negative lookahead
- Non-greedy quantifiers: `*?`, `+?`, `??`
- Character classes: `[^()]*` matches any character except parentheses

**Oniguruma vs PCRE differences:**
- Supports variable-length lookbehind (PCRE does not)
- Some syntax details differ — refer to Oniguruma documentation

### Matching Behavior

Multiple patterns within the same state are compiled into a single combined regex (joined with `|`). Matching follows Oniguruma's **leftmost-first** principle. When multiple patterns can match at the same position, **rules listed earlier have higher priority**.

Therefore, more specific rules should be placed before more general ones. For example:

```json
[
  { "pattern": "\\b(class)\\b${whiteSpace}+(${identifier})", "styles": [1, "keyword", 2, "class"] },
  { "pattern": "\\b(class)\\b", "styles": [1, "keyword"] }
]
```

---

## Style System

### Option 1: `style` - Whole Match Style

Applies a single style to the entire matched text:

```json
{
  "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"",
  "style": "string"
}
```

### Option 2: `styles` - Capture Group Style Mapping

Maps different capture groups to different styles. Format: `[groupNumber, "styleName", groupNumber, "styleName", ...]`:

```json
{
  "pattern": "\\b(class)\\b${whiteSpace}+(${identifier})",
  "styles": [1, "keyword", 2, "class"]
}
```

- Group numbers start from **1** (corresponding to the first `()` capture group in the regex)
- Matched text not covered by any capture group will not be highlighted
- Gap text between capture groups will not be highlighted

### Style Names

SweetLine does not have a predefined style list — style names are entirely user-defined. Common conventions include:

| Name | Typical Usage | Name | Typical Usage |
|------|--------------|------|--------------|
| `keyword` | Language keywords | `string` | String literals |
| `number` | Numeric literals | `comment` | Comments |
| `class` | Class/type names | `method` | Method/function names |
| `variable` | Variable names | `property` | Property names |
| `punctuation` | Punctuation/operators | `annotation` | Annotations/decorators |
| `builtin` | Built-in constants | `preprocessor` | Preprocessor directives |

When using style ID mode, register name-to-ID mappings via `engine.registerStyleName("keyword", 1)`.

---

## State Transitions

### `state` - Transition After Match

Automatically transitions to the specified state after a successful match. This is the core mechanism for handling cross-line syntax structures.

**Typical Scenario: String State**

```json
{
  "states": {
    "default": [
      {
        "pattern": "\"",
        "style": "string",
        "state": "doubleString"
      }
    ],
    "doubleString": [
      {
        "pattern": "\\\\.",
        "style": "string"
      },
      {
        "pattern": "\"",
        "style": "string",
        "state": "default"
      },
      {
        "pattern": "${any}",
        "style": "string"
      }
    ]
  }
}
```

Workflow:
1. In the `default` state, when `"` is encountered, it is styled as `string` and the state transitions to `doubleString`
2. In `doubleString`, `\\.` matches escape characters, keeping the string style
3. When the next `"` is encountered, it is styled as string and transitions back to `default`
4. `${any}` matches any other character, maintaining the string style (including across lines)

### Zero-Width Matches and State Transitions

SweetLine supports zero-width matches (matches with length 0), useful for "look without consuming" state transitions:

```json
{
  "pattern": "^(?=[^ \\t])",
  "style": "string",
  "state": "default"
}
```

This rule matches the position at **the start of a line where the next character is not a space/tab** (zero-width). It only performs a state transition without consuming any characters. The engine has a built-in infinite loop prevention mechanism: at most one zero-width match is allowed at the same position.

---

## SubStates

`subStates` allows delegating the content of specific capture groups to another state for processing, instead of directly assigning a style. This is very useful for handling **nested structures** such as generic parameters.

### Format

```json
"subStates": [groupNumber, "stateName", groupNumber, "stateName", ...]
```

### Example: Generic Type Handling

```json
{
  "pattern": "(${identifier})${whiteSpace}*(<)([^()]*)(>)${whiteSpace}+(${identifier})",
  "styles": [1, "class", 2, "punctuation", 4, "punctuation", 5, "variable"],
  "subStates": [3, "genericType"]
}
```

For `List<String> names`:
- Group 1 `List` → `class` style
- Group 2 `<` → `punctuation` style
- Group 3 `String` → delegated to `genericType` state (not styled in `styles`)
- Group 4 `>` → `punctuation` style
- Group 5 `names` → `variable` style

### genericType State Definition

```json
"genericType": [
  {
    "pattern": "(${identifier})${whiteSpace}*(<)([^()]*)(>)",
    "styles": [1, "class", 2, "punctuation", 4, "punctuation"],
    "subStates": [3, "genericType"]
  },
  {
    "pattern": "${identifier}",
    "style": "class"
  },
  {
    "pattern": "[,\\[\\]?]",
    "style": "punctuation"
  }
]
```

`subStates` can recursively reference its own state, enabling nested generic processing (e.g., `Map<String, List<Integer>>`).

**Note:** Capture groups referenced by `subStates` should not appear in `styles`, otherwise `styles` will override the sub-state processing results.

---

## onLineEndState - Line End State

`onLineEndState` is a special rule that controls state transitions at the end of a line. When a line finishes analysis, if the current state contains an `onLineEndState`, the next line will begin analysis from the specified state.

### Syntax

```json
{
  "onLineEndState": "targetStateName"
}
```

### Typical Use Cases

**1. State persists to the next line (state preservation):**

```json
"classHeader": [
  { "pattern": "${identifier}", "style": "class" },
  { "pattern": "[\\{;]", "style": "punctuation", "state": "default" },
  { "onLineEndState": "classHeader" }
]
```

When a class declaration spans multiple lines, `onLineEndState` ensures the next line remains in the `classHeader` state.

**2. Line-end state fallback:**

```json
"methodParams": [
  { "pattern": "\\)", "style": "punctuation", "state": "default" },
  { "onLineEndState": "default" }
]
```

If `)` is not encountered on the current line, the next line automatically returns to the `default` state.

**Note:** `onLineEndState` should be placed as the **last element** in the state's rule array.

---

## blockPairs - Code Block Pairs

`blockPairs` defines code block start/end markers, used for editor folding and indent guides.

```json
{
  "blockPairs": [
    {
      "start": "{",
      "end": "}",
      "branches": ["case"]
    }
  ]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `start` | string | Code block start marker |
| `end` | string | Code block end marker |
| `branches` | string[] | Optional, branch keywords within the block (e.g., `case` in switch) |

---

## Inline Style Mode

In `inline_style` mode, style definitions are written directly in the JSON. Highlighting results include colors and font attributes directly, without the need for external style registration.

### styles Definition

```json
{
  "styles": [
    {
      "name": "keyword",
      "foreground": "#FF569CD6",
      "background": "#00000000",
      "tags": ["bold"]
    },
    {
      "name": "string",
      "foreground": "#FFBD63C5"
    },
    {
      "name": "comment",
      "foreground": "#FF60AE6F",
      "tags": ["italic"]
    }
  ]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Style name, corresponding to the name referenced in patterns |
| `foreground` | string | Foreground color in `#AARRGGBB` (ARGB) format |
| `background` | string | Background color in `#AARRGGBB` (ARGB) format, optional |
| `tags` | string[] | Font attribute tags, optional. Supports `"bold"`, `"italic"`, `"strikethrough"` |

**Note:** When using `inline_style` mode, set `inline_style = true` when creating the `HighlightEngine`.

---

## Complete Example

Here is a complete simplified Java syntax rule example:

```json
{
  "name": "java",
  "fileExtensions": [".java"],
  "variables": {
    "identifier": "[a-zA-Z_$][\\w$]*",
    "whiteSpace": "[ \\t\\f]",
    "any": "[\\S\\s]",
    "primitiveType": "void|boolean|byte|char|short|int|long|float|double"
  },
  "states": {
    "default": [
      {
        "pattern": "\\b(class|interface|enum)\\b${whiteSpace}+(${identifier})",
        "styles": [1, "keyword", 2, "class"],
        "state": "classHeader"
      },
      {
        "pattern": "\\b(new)\\b${whiteSpace}+(${identifier})${whiteSpace}*(<)([^()]*)(>)${whiteSpace}*(\\()",
        "styles": [1, "keyword", 2, "class", 3, "punctuation", 5, "punctuation", 6, "punctuation"],
        "subStates": [4, "genericType"]
      },
      {
        "pattern": "\\b(new)\\b${whiteSpace}+(${identifier})${whiteSpace}*(\\()",
        "styles": [1, "keyword", 2, "class", 3, "punctuation"]
      },
      {
        "pattern": "\\b(public|private|protected|static|final|abstract|return|if|else|for|while)\\b",
        "styles": [1, "keyword"]
      },
      {
        "pattern": "\\b(true|false|null)\\b",
        "styles": [1, "builtin"]
      },
      {
        "pattern": "@${identifier}",
        "style": "annotation"
      },
      {
        "pattern": "\\b(${primitiveType})\\b${whiteSpace}+(${identifier})${whiteSpace}*(\\()",
        "styles": [1, "keyword", 2, "method", 3, "punctuation"]
      },
      {
        "pattern": "\\b(${primitiveType})\\b${whiteSpace}+(${identifier})${whiteSpace}*([;=,)])",
        "styles": [1, "keyword", 2, "variable", 3, "punctuation"]
      },
      {
        "pattern": "\\b(${identifier})${whiteSpace}*(<)([^()]*)(>)${whiteSpace}+(${identifier})${whiteSpace}*([;=,)])",
        "styles": [1, "class", 2, "punctuation", 4, "punctuation", 5, "variable", 6, "punctuation"],
        "subStates": [3, "genericType"]
      },
      {
        "pattern": "\\b(${identifier})${whiteSpace}+(${identifier})${whiteSpace}*([;=,)])",
        "styles": [1, "class", 2, "variable", 3, "punctuation"]
      },
      {
        "pattern": "(${identifier})${whiteSpace}*(\\()",
        "styles": [1, "method", 2, "punctuation"]
      },
      {
        "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"",
        "style": "string"
      },
      {
        "pattern": "\\b[0-9][0-9_]*\\.?[0-9_]*(?:[eE][+-]?[0-9]+)?[fFdDlL]?\\b",
        "style": "number"
      },
      {
        "pattern": "/\\*",
        "style": "comment",
        "state": "blockComment"
      },
      {
        "pattern": "//${any}*",
        "style": "comment"
      },
      {
        "pattern": "[.()\\[\\]{}+\\-*/<>=!&|;:,?~^%@]",
        "style": "punctuation"
      }
    ],
    "classHeader": [
      {
        "pattern": "(<)([^()]*)(>)",
        "styles": [1, "punctuation", 3, "punctuation"],
        "subStates": [2, "genericType"]
      },
      {
        "pattern": "\\b(extends|implements)\\b",
        "styles": [1, "keyword"]
      },
      {
        "pattern": "${identifier}",
        "style": "class"
      },
      {
        "pattern": "[,:]",
        "style": "punctuation"
      },
      {
        "pattern": "\\{",
        "style": "punctuation",
        "state": "default"
      },
      { "onLineEndState": "classHeader" }
    ],
    "genericType": [
      {
        "pattern": "\\b(extends|super)\\b",
        "styles": [1, "keyword"]
      },
      {
        "pattern": "(${identifier})${whiteSpace}*(<)([^()]*)(>)",
        "styles": [1, "class", 2, "punctuation", 4, "punctuation"],
        "subStates": [3, "genericType"]
      },
      {
        "pattern": "${identifier}",
        "style": "class"
      },
      {
        "pattern": "[,\\[\\]?]",
        "style": "punctuation"
      }
    ],
    "blockComment": [
      {
        "pattern": "\\*/",
        "style": "comment",
        "state": "default"
      },
      {
        "pattern": "${any}",
        "style": "comment"
      }
    ]
  },
  "blockPairs": [
    { "start": "{", "end": "}" }
  ]
}
```

---

## Best Practices

### 1. Pattern Ordering

Rule ordering affects matching priority. The recommended order for `default` state rules:

1. **Class/struct declarations** (triggers classHeader state transition)
2. **`new` expressions** (constructor calls)
3. **import/using statements**
4. **Variable declaration keywords** (var, let, etc.)
5. **Preprocessor directives**
6. **Built-in type + method/property/variable declarations**
7. **General keywords**
8. **Built-in constants** (true/false/null)
9. **Annotations/decorators**
10. **Generic type + method/property/variable declarations**
11. **Simple type + method/property/variable declarations**
12. **Method calls** (fallback pattern)
13. **String literals** (multi-line strings requiring state transitions)
14. **Numeric literals**
15. **Comments** (block comments require state transitions)
16. **Operators and punctuation** (fallback pattern)

### 2. Avoid Greedy Matching Pitfalls

When handling generic parameters, use `[^()]*` instead of `.*` to prevent regex backtracking across too much content:

```json
// Recommended
"(${identifier})${whiteSpace}*(<)([^()]*)(>)"

// Avoid
"(${identifier})${whiteSpace}*(<)(.*)(>)"
```

When the same line contains multiple independent `<>` pairs (e.g., class inheritance declarations), use non-greedy matching `(.*?)`:

```json
"(<)(.*?)(>)"
```

### 3. Use onLineEndState Appropriately

- Multi-line structures (class declarations, function parameter lists) use `onLineEndState` to preserve state
- States that only need to persist until line end don't need `onLineEndState` (defaults back to default)
- Place `onLineEndState` at the end of the state's rule array

### 4. Use Variables to Reduce Repetition

Extract frequently used regex fragments as variables:

```json
{
  "variables": {
    "identifier": "[a-zA-Z_]\\w*",
    "whiteSpace": "[ \\t\\f]",
    "builtinType": "int|float|double|string|bool|void"
  }
}
```

### 5. State Design Principles

- Every state should have clear **entry conditions** and **exit conditions**
- Multi-line syntax structures (strings, comments) must use state transitions
- The last pattern in a state should be a fallback match (e.g., `${any}`) to prevent the engine from getting stuck on certain characters
- Avoid creating too many states — most languages can be handled with 5-10 states
