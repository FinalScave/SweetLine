# 语法配置规则
## 示例Json
```json
{
  "name": "java",
  "fileExtensions": [".java"],
  "variables": {
    "identifierStart": "[\\p{Han}\\w_$]+",
    "identifierPart": "[\\p{Han}\\w_$0-9]*",
    "identifier": "${identifierStart}${identifierPart}"
  },
  "states": {
    "default": [
      {
        "pattern": "\\b(class|interface|enum|package|import)\\b",
        "style": "keyword"
      },
      {
        "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"",
        "style": "string"
      },
      {
        "pattern": "(${identifier})\\(",
        "styles": [0, "method"]
      },
      {
        "pattern": "//.*",
        "style": "comment"
      },
      {
        "pattern": "/\\*",
        "style": "comment",
        "state": "longComment"
      }
    ],
    "longComment": [
      {
        "pattern": "\\s\\S",
        "style": "comment",
        "multiLine": true
      },
      {
        "pattern": "\\*/",
        "style": "comment",
        "state": "default"
      }
    ]
  }
}
```
## 简要说明
使用json格式进行声明，支持变量定义、状态切换、跨行匹配。根节点属性包含 "name"、"fileExtensions"、"variables"、"states"，其中"variables"为可选项，其余为必选项
### 根节点 "name"
name 表示语法规则的名称，可随意定义
### 根节点 "fileExtensions"
fileExtensions 表示匹配文件的后缀名，须按照要高亮的文件后缀名指定(如".h", ".cpp" 等)
### 根节点 "variables"
variables 表示变量定义，可将正则表达式定义为变量，在后续的states中进行引用
### 根节点 "states"
states 表示语法规则包含的状态，每个状态各包含自己的匹配规则，每个状态都必须指定名称
### "states" 下状态节点
states 下包含多个状态，定义格式如下：
```json
{
  "states": {
    // 默认状态名
    "default": [
      {
        "pattern": "正则表达式",
        "style": "style", // 指定style名称，适用于没有捕获组使用全匹配的情况
        "styles": [捕获组索引0, "style0", 捕获组索引1, "style1"], // 奇数位为捕获组索引，偶数位为捕获组对应的style
        "multiLine": //是否多行匹配(true/false)
        "state": "state1" // 匹配到此处时需要跳转的状态
      }
    ],
    "state1": {
      // 同上....
    }
  }
}
```
