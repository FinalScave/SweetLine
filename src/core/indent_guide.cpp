#include "internal_highlight.h"
#include "util.h"

namespace NS_SWEETLINE {

  // ===================================== IndentGuideAnalyzer ============================================
  int32_t IndentGuideAnalyzer::computeLeadingWhitespace(const U8String& text, int32_t tab_size) {
    int32_t columns = 0;
    for (size_t i = 0; i < text.size(); ++i) {
      char c = text[i];
      if (c == ' ') {
        columns++;
      } else if (c == '\t') {
        columns += tab_size - (columns % tab_size);
      } else {
        break;
      }
    }
    return columns;
  }

  bool IndentGuideAnalyzer::isStringOrCommentStyle(const U8String& style_name) {
    return style_name.find("string") != U8String::npos
      || style_name.find("comment") != U8String::npos
      || style_name.find("char") != U8String::npos;
  }

  void IndentGuideAnalyzer::analyzeByBlockPairs(
    const SharedPtr<SyntaxRule>& rule,
    const SharedPtr<Document>& document,
    const SharedPtr<DocumentHighlight>& highlight,
    SharedPtr<IndentGuideResult>& result) {

    if (highlight == nullptr || document == nullptr || rule == nullptr) {
      return;
    }

    struct BlockStackEntry {
      const ScopeRule* rule;
      int32_t start_line;
      int32_t start_column;
      List<IndentGuideLine::BranchPoint> branches;
    };

    List<BlockStackEntry> block_stack;
    size_t line_count = highlight->lines.size();
    result->line_states.resize(line_count);

    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      const LineHighlight& line_highlight = highlight->lines[line_num];
      const U8String& line_text = document->getLine(line_num).text;

      for (const TokenSpan& span : line_highlight.spans) {
        U8String token_text;
        if (!span.matched_text.empty()) {
          token_text = span.matched_text;
        } else {
          size_t start_col = span.range.start.column;
          size_t end_col = span.range.end.column;
          if (start_col < end_col) {
            token_text = Utf8Util::utf8Substr(line_text, start_col, end_col - start_col);
          }
        }
        if (token_text.empty()) {
          continue;
        }

        // 检查 style 名称来过滤字符串/注释中的标记
        // 用 style_id 反查 style_name（通过 inline_styles 或 StyleMapping）
        // 简化处理：style_id == 0 跳过（default 样式，即无高亮部分）
        // 对于注释和字符串类型的 style，通常 style 名称中包含 "string"/"comment"
        // 在 inline_style 模式下检查 rule 的 style_mapping；否则不好直接判断，
        // 所以这里用一个启发式方法：如果 rule 有 style_mapping 就反查名称
        bool skip = false;
        if (rule->style_mapping) {
          const U8String& name = rule->style_mapping->getStyleName(span.style_id);
          skip = isStringOrCommentStyle(name);
        }
        if (skip) {
          continue;
        }

        // 对每个 ScopeRule 进行匹配
        for (const auto& [rule_id, scope_rule] : rule->scope_rules_map) {
          // 检查 start
          if (token_text == scope_rule.start) {
            int32_t col = static_cast<int32_t>(span.range.start.column);
            block_stack.push_back({&scope_rule, static_cast<int32_t>(line_num), col, {}});
            // 更新行状态
            result->line_states[line_num].scope_state = ScopeState::START;
            result->line_states[line_num].nesting_level = static_cast<int32_t>(block_stack.size()) - 1;
            result->line_states[line_num].scope_column = col;
            goto next_span;
          }

          // 检查 end
          if (!scope_rule.end.empty() && token_text == scope_rule.end && !block_stack.empty()) {
            // 从栈顶找到匹配的 rule
            for (int32_t si = static_cast<int32_t>(block_stack.size()) - 1; si >= 0; --si) {
              if (block_stack[si].rule->rule_id == scope_rule.rule_id) {
                BlockStackEntry entry = std::move(block_stack[si]);
                block_stack.erase(block_stack.begin() + si);

                IndentGuideLine guide;
                guide.column = entry.start_column;
                guide.start_line = entry.start_line;
                guide.end_line = static_cast<int32_t>(line_num);
                guide.nesting_level = si;
                guide.scope_rule_id = scope_rule.rule_id;
                guide.branches = std::move(entry.branches);
                result->guide_lines.push_back(std::move(guide));

                // 更新行状态
                result->line_states[line_num].scope_state = ScopeState::END;
                result->line_states[line_num].nesting_level = si;
                result->line_states[line_num].scope_column = entry.start_column;
                goto next_span;
              }
            }
          }

          // 检查 branch
          if (!block_stack.empty() && scope_rule.branch_keywords.find(token_text) != scope_rule.branch_keywords.end()) {
            // 从栈顶找到匹配的 rule
            for (int32_t si = static_cast<int32_t>(block_stack.size()) - 1; si >= 0; --si) {
              if (block_stack[si].rule->rule_id == scope_rule.rule_id) {
                block_stack[si].branches.push_back({static_cast<int32_t>(line_num), static_cast<int32_t>(span.range.start.column)});
                goto next_span;
              }
            }
          }
        }

        next_span:;
      }

      // 设置中间行的 nesting_level
      if (result->line_states[line_num].scope_state == ScopeState::CONTENT) {
        result->line_states[line_num].nesting_level = static_cast<int32_t>(block_stack.size());
      }
    }

    // 处理未闭合的块（文件末尾未关闭的 { ）
    for (auto& entry : block_stack) {
      IndentGuideLine guide;
      guide.column = entry.start_column;
      guide.start_line = entry.start_line;
      guide.end_line = static_cast<int32_t>(line_count) - 1;
      guide.nesting_level = 0;
      guide.scope_rule_id = entry.rule->rule_id;
      guide.branches = std::move(entry.branches);
      result->guide_lines.push_back(std::move(guide));
    }
  }

  void IndentGuideAnalyzer::analyzeByIndentation(
    const SharedPtr<Document>& document,
    int32_t tab_size,
    SharedPtr<IndentGuideResult>& result) {

    if (document == nullptr || tab_size <= 0) {
      return;
    }

    size_t line_count = document->getLineCount();
    if (line_count == 0) {
      return;
    }

    // 计算每行的缩进列数
    List<int32_t> indent_columns(line_count, 0);
    List<bool> is_empty_line(line_count, false);
    for (size_t i = 0; i < line_count; ++i) {
      const U8String& text = document->getLine(i).text;
      if (text.empty() || text.find_first_not_of(" \t") == U8String::npos) {
        is_empty_line[i] = true;
        indent_columns[i] = -1;
      } else {
        indent_columns[i] = computeLeadingWhitespace(text, tab_size);
      }
    }

    // 空行插值（空行的缩进列数 = min(上一非空行, 下一非空行)）
    for (size_t i = 0; i < line_count; ++i) {
      if (!is_empty_line[i]) continue;
      int32_t prev_indent = 0;
      for (int32_t p = static_cast<int32_t>(i) - 1; p >= 0; --p) {
        if (!is_empty_line[p]) {
          prev_indent = indent_columns[p];
          break;
        }
      }
      int32_t next_indent = 0;
      for (size_t n = i + 1; n < line_count; ++n) {
        if (!is_empty_line[n]) {
          next_indent = indent_columns[n];
          break;
        }
      }
      indent_columns[i] = std::min(prev_indent, next_indent);
    }

    // 计算缩进等级并填充 line_states
    int32_t max_indent_level = 0;
    result->line_states.resize(line_count);
    for (size_t i = 0; i < line_count; ++i) {
      int32_t level = indent_columns[i] / tab_size;
      result->line_states[i].indent_level = level;
      result->line_states[i].nesting_level = level;
      if (level > max_indent_level) {
        max_indent_level = level;
      }
    }

    // 生成纵向划线——对每个缩进等级扫描连续区间
    for (int32_t level = 1; level <= max_indent_level; ++level) {
      int32_t col = level * tab_size;
      int32_t scan_start = -1;
      for (size_t line = 0; line <= line_count; ++line) {
        bool in_range = (line < line_count) && (indent_columns[line] >= col);
        if (in_range) {
          if (scan_start < 0) {
            scan_start = static_cast<int32_t>(line);
          }
        } else {
          if (scan_start >= 0) {
            IndentGuideLine guide;
            guide.column = col;
            guide.start_line = scan_start;
            guide.end_line = static_cast<int32_t>(line) - 1;
            guide.nesting_level = level - 1;
            guide.scope_rule_id = -1;
            result->guide_lines.push_back(std::move(guide));
            scan_start = -1;
          }
        }
      }
    }
  }

  void IndentGuideAnalyzer::analyzeByIndentationWithStart(
    const SharedPtr<SyntaxRule>& rule,
    const SharedPtr<Document>& document,
    const SharedPtr<DocumentHighlight>& highlight,
    int32_t tab_size,
    SharedPtr<IndentGuideResult>& result) {

    if (document == nullptr || highlight == nullptr || tab_size <= 0) {
      return;
    }

    size_t line_count = document->getLineCount();
    if (line_count == 0) {
      return;
    }

    // 先计算每行的缩进列数
    List<int32_t> indent_columns(line_count, 0);
    List<bool> is_empty_line(line_count, false);
    for (size_t i = 0; i < line_count; ++i) {
      const U8String& text = document->getLine(i).text;
      if (text.empty() || text.find_first_not_of(" \t") == U8String::npos) {
        is_empty_line[i] = true;
        indent_columns[i] = -1;
      } else {
        indent_columns[i] = computeLeadingWhitespace(text, tab_size);
      }
    }

    // 空行插值
    for (size_t i = 0; i < line_count; ++i) {
      if (!is_empty_line[i]) continue;
      int32_t prev_indent = 0;
      for (int32_t p = static_cast<int32_t>(i) - 1; p >= 0; --p) {
        if (!is_empty_line[p]) {
          prev_indent = indent_columns[p];
          break;
        }
      }
      int32_t next_indent = 0;
      for (size_t n = i + 1; n < line_count; ++n) {
        if (!is_empty_line[n]) {
          next_indent = indent_columns[n];
          break;
        }
      }
      indent_columns[i] = std::min(prev_indent, next_indent);
    }

    result->line_states.resize(line_count);

    // 找到 indent-based scope rules (end == "")
    List<const ScopeRule*> indent_scope_rules;
    for (const auto& [rule_id, scope_rule] : rule->scope_rules_map) {
      if (scope_rule.end.empty()) {
        indent_scope_rules.push_back(&scope_rule);
      }
    }

    // 对每一行，检查是否有 start token (如 ":")
    // 如果有，从下一行开始找到缩进回退的位置作为 block end
    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      const LineHighlight& line_highlight = highlight->lines[line_num];
      const U8String& line_text = document->getLine(line_num).text;

      for (const TokenSpan& span : line_highlight.spans) {
        U8String token_text;
        if (!span.matched_text.empty()) {
          token_text = span.matched_text;
        } else {
          size_t start_col = span.range.start.column;
          size_t end_col = span.range.end.column;
          if (start_col < end_col) {
            token_text = Utf8Util::utf8Substr(line_text, start_col, end_col - start_col);
          }
        }
        if (token_text.empty()) continue;

        // 跳过注释/字符串
        bool skip = false;
        if (rule->style_mapping) {
          const U8String& name = rule->style_mapping->getStyleName(span.style_id);
          skip = isStringOrCommentStyle(name);
        }
        if (skip) continue;

        for (const ScopeRule* br : indent_scope_rules) {
          if (token_text == br->start) {
            int32_t start_indent = indent_columns[line_num];
            // 从下一行开始，找到缩进 <= start_indent 的第一个非空行
            int32_t end_line = static_cast<int32_t>(line_num);
            for (size_t n = line_num + 1; n < line_count; ++n) {
              if (is_empty_line[n]) continue;
              if (indent_columns[n] <= start_indent) {
                break;
              }
              end_line = static_cast<int32_t>(n);
            }

            if (end_line > static_cast<int32_t>(line_num)) {
              IndentGuideLine guide;
              guide.column = start_indent + tab_size;
              guide.start_line = static_cast<int32_t>(line_num) + 1;
              guide.end_line = end_line;
              guide.nesting_level = start_indent / tab_size;
              guide.scope_rule_id = br->rule_id;
              result->guide_lines.push_back(std::move(guide));

              result->line_states[line_num].scope_state = ScopeState::START;
              result->line_states[line_num].nesting_level = start_indent / tab_size;
              result->line_states[line_num].scope_column = start_indent;
            }
            goto next_line;
          }
        }
      }
      next_line:;
    }

    // 补充缩进等级信息
    for (size_t i = 0; i < line_count; ++i) {
      result->line_states[i].indent_level = indent_columns[i] / tab_size;
      if (result->line_states[i].nesting_level < 0) {
        result->line_states[i].nesting_level = result->line_states[i].indent_level;
      }
    }
  }
}
