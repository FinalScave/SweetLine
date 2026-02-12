# SweetLine 语法规则配置文档

SweetLine 使用 JSON 格式定义语法高亮规则。每个 JSON 文件描述一种编程语言的高亮规则，引擎在运行时编译这些规则并用于文本分析。

## 目录

- [基本结构](#基本结构)
- [顶层字段](#顶层字段)
- [variables - 变量定义](#variables---变量定义)
- [states - 状态机定义](#states---状态机定义)
- [Pattern 匹配规则](#pattern-匹配规则)
- [样式系统](#样式系统)
- [状态转换](#状态转换)
- [子状态 subStates](#子状态-substates)
- [行尾状态 onLineEndState](#行尾状态-onlineendstate)
- [blockPairs - 代码块配对](#blockpairs---代码块配对)
- [内联样式模式 styles](#内联样式模式-styles)
- [完整示例](#完整示例)
- [最佳实践](#最佳实践)

---

## 基本结构

一个完整的语法规则 JSON 文件的基本结构如下：

```json
{
  "name": "语言名称",
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

## 顶层字段

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 语法规则名称，用于 `getSyntaxRuleByName()` 查找 |
| `fileExtensions` | string[] | 是 | 支持的文件扩展名列表，用于 `getSyntaxRuleByExtension()` 匹配 |
| `variables` | object | 否 | 可复用的正则表达式变量定义 |
| `styles` | array | 否 | 内联样式定义（仅 `inline_style` 模式使用） |
| `states` | object | 是 | 状态机定义，包含所有状态及其匹配规则 |
| `blockPairs` | array | 否 | 代码块配对定义（用于折叠/缩进导引线） |

---

## variables - 变量定义

`variables` 用于定义可复用的正则表达式片段，在 `pattern` 中通过 `${变量名}` 引用。变量可以引用其他变量（支持嵌套展开）。

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

**在 pattern 中使用：**

```json
{
  "pattern": "\\b(${primitiveType})\\b${whiteSpace}+(${identifier})",
  "styles": [1, "keyword", 2, "variable"]
}
```

展开后等价于：
```
\b(void|boolean|byte|char|short|int|long|float|double)\b[ \t\f]+([a-zA-Z_]\w*)
```

**注意事项：**
- 变量名区分大小写
- 变量支持嵌套引用（如 `"identifier": "${identifierStart}${identifierPart}"`）
- SweetLine 使用 Oniguruma 正则语法，支持 `\p{Han}`（Unicode 属性）、零宽断言等高级特性
- JSON 中的反斜杠需要双重转义：正则 `\b` 在 JSON 中写作 `\\b`

---

## states - 状态机定义

`states` 是语法规则的核心，定义了一个有限状态机（FSM）。每个状态包含一组有序的匹配规则，引擎按顺序尝试匹配，使用第一个成功匹配的规则。

### 必须包含 `default` 状态

`default` 是初始状态，所有文本解析从此状态开始。其他状态通过 `state` 字段切换进入。

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

### 状态中的规则

每个状态是一个规则数组 `[]`，数组中的每个元素是一个**匹配规则对象**。引擎在当前位置按数组顺序逐一尝试匹配，匹配成功后应用样式并移动到匹配文本之后的位置。

---

## Pattern 匹配规则

每个匹配规则对象包含以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `pattern` | string | 是* | Oniguruma 正则表达式，支持 `${变量名}` 替换 |
| `style` | string | 否 | 整个匹配的样式名称（与 `styles` 二选一） |
| `styles` | array | 否 | 捕获组样式映射（与 `style` 二选一） |
| `state` | string | 否 | 匹配成功后切换到的目标状态 |
| `subStates` | array | 否 | 将指定捕获组内容交给子状态处理 |
| `onLineEndState` | string | 否* | 行尾自动切换状态（特殊规则，不含 pattern） |

> `*` 注：普通匹配规则必须有 `pattern`；行尾状态规则只有 `onLineEndState`，不含 `pattern`。

### 基本 pattern 示例

```json
{
  "pattern": "\\b(if|else|while|for|return)\\b",
  "styles": [1, "keyword"]
}
```

### 正则表达式说明

SweetLine 使用 [Oniguruma](https://github.com/kkos/oniguruma) 作为正则引擎，支持以下特性：

- 标准正则：`\b`, `\w`, `\d`, `\s`, `.`, `*`, `+`, `?`, `|`, `()`, `[]` 等
- Unicode 属性：`\p{Han}` (汉字), `\p{L}` (字母) 等
- 零宽断言：`(?=...)` 前瞻, `(?<=...)` 后顾, `(?!...)` 否定前瞻
- 非贪婪量词：`*?`, `+?`, `??`
- 字符类：`[^()]*` 匹配不含括号的任意字符

**Oniguruma 与 PCRE 的区别：**
- 支持变长后顾断言（PCRE 不支持）
- 部分语法细节不同，建议参阅 Oniguruma 文档

### 匹配规则说明

同一状态中的多个 pattern 编译后会被合并为一个大正则（使用 `|` 连接），匹配时由 Oniguruma 引擎按**最左优先**原则匹配。当多个 pattern 在相同位置都能匹配时，**排列在前面的规则优先级更高**。

因此，更具体的规则应该放在更通用的规则前面。例如：

```json
[
  { "pattern": "\\b(class)\\b${whiteSpace}+(${identifier})", "styles": [1, "keyword", 2, "class"] },
  { "pattern": "\\b(class)\\b", "styles": [1, "keyword"] }
]
```

---

## 样式系统

### 方式一：`style` - 整体样式

将整个匹配文本应用同一个样式：

```json
{
  "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"",
  "style": "string"
}
```

### 方式二：`styles` - 捕获组样式映射

将不同捕获组映射到不同样式，格式为 `[组号, "样式名", 组号, "样式名", ...]`：

```json
{
  "pattern": "\\b(class)\\b${whiteSpace}+(${identifier})",
  "styles": [1, "keyword", 2, "class"]
}
```

- 组号从 **1** 开始（对应正则中的第一个 `()` 捕获组）
- 未被任何捕获组覆盖的匹配文本不会被高亮
- 捕获组之间的间隔文本（gap）也不会被高亮

### 内置样式名称

SweetLine 没有预定义样式列表，样式名称完全由使用者自行定义。常用的约定名称包括：

| 名称 | 典型用途 | 名称 | 典型用途 |
|------|----------|------|----------|
| `keyword` | 语言关键字 | `string` | 字符串字面量 |
| `number` | 数字字面量 | `comment` | 注释 |
| `class` | 类/类型名 | `method` | 方法/函数名 |
| `variable` | 变量名 | `property` | 属性名 |
| `punctuation` | 标点/运算符 | `annotation` | 注解/装饰器 |
| `builtin` | 内置常量 | `preprocessor` | 预处理器指令 |

使用样式 ID 模式时，需要通过 `engine.registerStyleName("keyword", 1)` 注册名称到 ID 的映射。

---

## 状态转换

### `state` - 匹配后切换状态

匹配成功后自动切换到指定状态。这是实现跨行语法结构的核心机制。

**典型场景：字符串状态**

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

工作流程：
1. 在 `default` 状态遇到 `"` 时，标记为 `string` 样式，切换到 `doubleString` 状态
2. 在 `doubleString` 中，`\\.` 匹配转义字符，保持 string 样式
3. 遇到下一个 `"` 时，标记为 string，切换回 `default`
4. `${any}` 匹配任意其他字符，保持 string 样式（包括跨行）

### 零宽匹配与状态切换

SweetLine 支持零宽匹配（匹配长度为 0），这在需要"查看但不消耗"的状态切换中非常有用：

```json
{
  "pattern": "^(?=[^ \\t])",
  "style": "string",
  "state": "default"
}
```

此规则匹配**行首且下一个字符不是空格/制表符**的位置（零宽），仅执行状态切换但不消耗任何字符。引擎内置了零宽匹配的防无限循环机制：同一位置最多允许一次零宽匹配。

---

## 子状态 subStates

`subStates` 允许将特定捕获组的内容交给另一个状态来处理，而不是直接赋予样式。这对于处理**泛型参数**等嵌套结构非常有用。

### 格式

```json
"subStates": [捕获组号, "状态名", 捕获组号, "状态名", ...]
```

### 示例：泛型类型处理

```json
{
  "pattern": "(${identifier})${whiteSpace}*(<)([^()]*)(>)${whiteSpace}+(${identifier})",
  "styles": [1, "class", 2, "punctuation", 4, "punctuation", 5, "variable"],
  "subStates": [3, "genericType"]
}
```

对于 `List<String> names`：
- 组1 `List` → `class` 样式
- 组2 `<` → `punctuation` 样式
- 组3 `String` → 交给 `genericType` 状态处理（不在 styles 中赋予样式）
- 组4 `>` → `punctuation` 样式
- 组5 `names` → `variable` 样式

### genericType 状态定义

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

`subStates` 可以递归使用自身状态，实现嵌套泛型的处理（如 `Map<String, List<Integer>>`）。

**注意：** 被 `subStates` 引用的捕获组不应出现在 `styles` 中，否则 `styles` 会覆盖子状态的处理结果。

---

## 行尾状态 onLineEndState

`onLineEndState` 是一种特殊规则，用于控制行尾时的状态切换。当一行分析结束时，如果当前状态中存在 `onLineEndState`，下一行将从指定状态开始分析。

### 语法

```json
{
  "onLineEndState": "目标状态名"
}
```

### 典型用途

**1. 状态持续到下一行（状态保持）：**

```json
"classHeader": [
  { "pattern": "${identifier}", "style": "class" },
  { "pattern": "[\\{;]", "style": "punctuation", "state": "default" },
  { "onLineEndState": "classHeader" }
]
```

当类声明跨越多行时，`onLineEndState` 确保下一行仍在 `classHeader` 状态中。

**2. 行尾状态回退：**

```json
"methodParams": [
  { "pattern": "\\)", "style": "punctuation", "state": "default" },
  { "onLineEndState": "default" }
]
```

如果在当前行没有遇到 `)`，下一行自动回到 `default` 状态。

**注意：** `onLineEndState` 应放在状态规则数组的**最后一个元素**。

---

## blockPairs - 代码块配对

`blockPairs` 定义代码块的起止标记，用于编辑器的折叠功能和缩进导引线。

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

| 字段 | 类型 | 说明 |
|------|------|------|
| `start` | string | 代码块开始标记 |
| `end` | string | 代码块结束标记 |
| `branches` | string[] | 可选，代码块内的分支关键字（如 switch 的 case） |

---

## 内联样式模式 styles

在 `inline_style` 模式下，样式定义直接写在 JSON 中，高亮结果直接包含颜色和字体属性，无需外部注册样式映射。

### styles 定义

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

| 字段 | 类型 | 说明 |
|------|------|------|
| `name` | string | 样式名称，与 pattern 中引用的名称对应 |
| `foreground` | string | 前景色，格式为 `#AARRGGBB`（ARGB） |
| `background` | string | 背景色，格式为 `#AARRGGBB`（ARGB），可选 |
| `tags` | string[] | 字体属性标签，可选，支持 `"bold"`, `"italic"`, `"strikethrough"` |

**注意：** 使用 `inline_style` 模式时，需要在创建 `HighlightEngine` 时设置 `inline_style = true`。

---

## 完整示例

以下是一个完整的简化版 Java 语法规则示例：

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

## 最佳实践

### 1. Pattern 排列顺序

规则的排列顺序影响匹配优先级，建议按以下顺序组织 `default` 状态的规则：

1. **类/结构体声明**（触发 classHeader 状态切换）
2. **`new` 表达式**（构造器调用）
3. **import/using 语句**
4. **变量声明关键字**（var、let 等）
5. **预处理器指令**
6. **内置类型 + 方法/属性/变量声明**
7. **通用关键字**
8. **内置常量**（true/false/null）
9. **注解/装饰器**
10. **泛型类型 + 方法/属性/变量声明**
11. **简单类型 + 方法/属性/变量声明**
12. **方法调用**（兜底模式）
13. **字符串字面量**（需要状态切换的多行字符串）
14. **数字字面量**
15. **注释**（块注释需要状态切换）
16. **运算符和标点**（兜底模式）

### 2. 避免贪婪匹配陷阱

在处理泛型参数时，使用 `[^()]*` 而非 `.*`，避免正则回溯跨越过多内容：

```json
// 推荐
"(${identifier})${whiteSpace}*(<)([^()]*)(>)"

// 避免
"(${identifier})${whiteSpace}*(<)(.*)(>)"
```

当同一行有多个独立的 `<>` 对（如类继承声明）时，使用非贪婪匹配 `(.*?)`：

```json
"(<)(.*?)(>)"
```

### 3. 合理使用 onLineEndState

- 多行结构（类声明、函数参数列表）使用 `onLineEndState` 保持状态
- 只需要保持到行尾的状态不需要 `onLineEndState`（默认回到 default）
- `onLineEndState` 放在状态规则数组的最后

### 4. 使用 variables 减少重复

将频繁使用的正则片段提取为变量：

```json
{
  "variables": {
    "identifier": "[a-zA-Z_]\\w*",
    "whiteSpace": "[ \\t\\f]",
    "builtinType": "int|float|double|string|bool|void"
  }
}
```

### 5. 状态设计原则

- 每个状态应有明确的**进入条件**和**退出条件**
- 多行语法结构（字符串、注释）必须使用状态切换
- 状态内的最后一个 pattern 应该是兜底匹配（如 `${any}`），防止引擎在某些字符上无法推进
- 避免创建过多状态，大多数语言 5-10 个状态即可满足需求
