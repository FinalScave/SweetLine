English | [简体中文](./README.md)
# SweetLine Highlight Engine

## Overview
SweetLine is a cross-platform, high-performance, and extensible syntax highlighting engine designed for modern code editors. It employs advanced regular expression matching technology and incremental update algorithms to process large code files in real-time and provide accurate syntax highlighting.

## Core Features
### 🚀 High Performance
- Based on the Oniguruma regex engine for fast pattern matching
- Incremental update algorithm that only reanalyzes changed parts
- Multi-line state preservation to avoid full document reanalysis
### 🎯 High Accuracy
- Supports complex syntax rule nesting
- Multi-state automata support (e.g., strings, comments, and other contextual states)
- Multiple capture group style mapping
### 🔧 Highly Extensible
- Supports JSON configuration for syntax rules
- Supports variable substitution and pattern reuse
### 📦 Modern Design
- C++17 standard, type-safe
- Clear API design

## Quick Start
### Basic Usage
```c++
#include "highlight.h"

using namespace NS_SWEETLINE;

// Create highlight engine
Ptr<HighlightEngine> engine = MAKE_PTR<HighlightEngine>();
// Compile syntax rules
Ptr<SyntaxRule> syntax_rule = engine->compileSyntaxFromFile("java_syntax.json");
// Create document object
Ptr<Document> document = std::make_shared<Document>("file:///example.java", R"(
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
)");
// Load document object and perform analysis
Ptr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
Ptr<DocumentHighlight> highlight = analyzer->analyze();
```

### Custom Syntax Rules
Refer to [Syntax Configuration Specification](docs/syntax_rule.md) for examples
```json
{
  "name": "Java",
  "file_extensions": [
    ".java"
  ],
  "states": {
    "default": [
      {
        "pattern": "\\b(public|private|class|static)\\b",
        "styles": [
          1,
          "keyword"
        ]
      },
      {
        "pattern": "\"",
        "styles": "string",
        "state": "quotedString"
      }
    ],
    "quotedString": [
      {
        "pattern": "\"",
        "style": "string",
        "state": "default"
      },
      {
        "pattern": "[^\"]*",
        "style": "string"
      }
    ]
  }
}
```

### Incremental Updates
```c++
TextRange change_range { {2, 4}, {2, 8} };
String new_text = "modified";
// Only reanalyze the changed parts
Ptr<DocumentHighlight> new_highlight = analyzer->analyzeChanges(change_range, new_text);
```

### Highlight Style Mapping
```c++
// Register custom styles
engine->registerStyleName("keyword", 1);
engine->registerStyleName("number", 2);
engine->registerStyleName("string", 3);
// Get style name
const String& style_name = engine->getStyleName(1); // Returns "keyword"
```

## Advanced Features
### Multi-syntax Support
```c++
// Compile multiple syntax rules
Ptr<SyntaxRule> java_rule = engine->compileSyntaxFromFile("java.json");
Ptr<SyntaxRule> cpp_rule = engine->compileSyntaxFromFile("cpp.json");
Ptr<SyntaxRule> python_rule = engine->compileSyntaxFromFile("python.json");
// Get syntax rules by file extension
Ptr<Document> document = MAKE_PTR<Document>("file:///example.py", "print('Hello')");
Ptr<SyntaxRule> syntax = engine->getSyntaxRuleByExtension(".py");
// Future support for referencing compiled syntax rules within syntax rule configurations
// TODO: Reference compiled syntax rules in syntax rule configuration files
```
### Performance Recommendations
- Pre-compile syntax rules: Compile all required syntax rules at application startup
- Use incremental updates appropriately: For large files, prioritize incremental updates over full analysis
- Optimize regular expressions: Avoid overly complex patterns, use variables to reuse common patterns
- Batch updates: Merge consecutive small changes into a single incremental update

## Native Platform Integration
### Android Integration
Android provides convenient JNI bindings with class and function names consistent with the C++ side. You can directly depend on the source code or import from maven:
```groovy
implementation 'com.qiplat:sweetline:0.0.4'
```

## Contributing
We welcome contributions from all developers! If you're interested in participating in the project, feel free to fork the repository, make changes, and submit merge requests. For project collaboration guidelines, please refer to [Project Collaboration Guide](docs/join.md)