#include <algorithm>

#include "internal_highlight.h"
#include "util.h"

namespace NS_SWEETLINE {
  namespace {
    U8String extractScopeTokenText(const U8String& line_text, const TokenSpan& span) {
      size_t start_col = span.range.start.column;
      size_t end_col = span.range.end.column;
      if (start_col >= end_col) {
        return {};
      }
      return Utf8Util::utf8Substr(line_text, start_col, end_col - start_col);
    }

    bool shouldSkipScopeToken(const SharedPtr<SyntaxRule>& rule, const TokenSpan& span) {
      return rule->scope_skip_style_ids.find(span.style_id) != rule->scope_skip_style_ids.end();
    }

    List<const ScopeRule*> collectScopeRulesOrdered(const SharedPtr<SyntaxRule>& rule, bool indent_only) {
      List<const ScopeRule*> ordered_rules;
      ordered_rules.reserve(rule->scope_rules_map.size());
      for (const auto& pair : rule->scope_rules_map) {
        const ScopeRule& scope_rule = pair.second;
        if (indent_only && !scope_rule.end.empty()) {
          continue;
        }
        ordered_rules.push_back(&scope_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const ScopeRule* lhs, const ScopeRule* rhs) {
          return lhs->rule_id < rhs->rule_id;
        });
      return ordered_rules;
    }
  }

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

  void IndentGuideAnalyzer::analyzeByScopeRules(
    const SharedPtr<SyntaxRule>& rule,
    const SharedPtr<Document>& document,
    const SharedPtr<DocumentHighlight>& highlight,
    SharedPtr<IndentGuideResult>& result) {

    if (highlight == nullptr || document == nullptr || rule == nullptr) {
      return;
    }

    struct ScopeStackEntry {
      const ScopeRule* rule;
      int32_t start_line;
      int32_t start_column;
      List<IndentGuideLine::BranchPoint> branches;
    };

    List<const ScopeRule*> ordered_scope_rules = collectScopeRulesOrdered(rule, false);
    List<ScopeStackEntry> block_stack;
    size_t line_count = highlight->lines.size();
    result->line_states.resize(line_count);

    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      const LineHighlight& line_highlight = highlight->lines[line_num];
      const U8String& line_text = document->getLine(line_num).text;

      for (const TokenSpan& span : line_highlight.spans) {
        U8String token_text = extractScopeTokenText(line_text, span);
        int32_t span_start_column = static_cast<int32_t>(span.range.start.column);
        if (token_text.empty()) {
          continue;
        }
        if (shouldSkipScopeToken(rule, span)) {
          continue;
        }
        auto handleScopeToken = [&](const U8String& token, int32_t token_column) {
          bool handled = false;
          if (!block_stack.empty()) {
            for (int32_t si = static_cast<int32_t>(block_stack.size()) - 1; si >= 0; --si) {
              const ScopeRule* active_rule = block_stack[si].rule;
              if (active_rule->end.empty() || token != active_rule->end) {
                continue;
              }
              ScopeStackEntry entry = std::move(block_stack[si]);
              block_stack.erase(block_stack.begin() + si);
              int32_t guide_column = std::min(entry.start_column, token_column);

              IndentGuideLine guide;
              guide.column = guide_column;
              guide.start_line = entry.start_line;
              guide.end_line = static_cast<int32_t>(line_num);
              guide.nesting_level = si;
              guide.scope_rule_id = active_rule->rule_id;
              guide.branches = std::move(entry.branches);
              result->guide_lines.push_back(std::move(guide));

              result->line_states[line_num].scope_state = ScopeState::END;
              result->line_states[line_num].nesting_level = si;
              result->line_states[line_num].scope_column = guide_column;
              handled = true;
              break;
            }
          }
          if (handled) {
            return true;
          }

          if (!block_stack.empty()) {
            for (int32_t si = static_cast<int32_t>(block_stack.size()) - 1; si >= 0; --si) {
              const ScopeRule* active_rule = block_stack[si].rule;
              if (active_rule->branch_keywords.find(token) == active_rule->branch_keywords.end()) {
                continue;
              }
              block_stack[si].branches.push_back({static_cast<int32_t>(line_num), token_column});
              return true;
            }
          }

          for (const ScopeRule* scope_rule : ordered_scope_rules) {
            if (token != scope_rule->start) {
              continue;
            }
            block_stack.push_back({scope_rule, static_cast<int32_t>(line_num), token_column, {}});
            result->line_states[line_num].scope_state = ScopeState::START;
            result->line_states[line_num].nesting_level = static_cast<int32_t>(block_stack.size()) - 1;
            result->line_states[line_num].scope_column = token_column;
            return true;
          }
          return false;
        };

        bool handled = handleScopeToken(token_text, span_start_column);
        if (!handled) {
          size_t token_char_count = Utf8Util::countChars(token_text);
          if (token_char_count > 1) {
            for (size_t i = 0; i < token_char_count; ++i) {
              U8String one_char = Utf8Util::utf8Substr(token_text, i, 1);
              if (one_char.empty()) {
                continue;
              }
              handleScopeToken(one_char, span_start_column + static_cast<int32_t>(i));
            }
          }
        }
      }

      // 设置中间行的 nesting_level
      if (result->line_states[line_num].scope_state == ScopeState::CONTENT) {
        result->line_states[line_num].nesting_level = static_cast<int32_t>(block_stack.size());
      }
    }

    // 处理未闭合的块（文件末尾未关闭的 { ）
    for (size_t i = 0; i < block_stack.size(); ++i) {
      auto& entry = block_stack[i];
      IndentGuideLine guide;
      guide.column = entry.start_column;
      guide.start_line = entry.start_line;
      guide.end_line = static_cast<int32_t>(line_count) - 1;
      guide.nesting_level = static_cast<int32_t>(i);
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
    List<const ScopeRule*> indent_scope_rules = collectScopeRulesOrdered(rule, true);

    // 对每一行，检查是否有 start token (如 ":")
    // 如果有，从下一行开始找到缩进回退的位置作为 block end
    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      const LineHighlight& line_highlight = highlight->lines[line_num];
      const U8String& line_text = document->getLine(line_num).text;
      bool matched_start = false;

      for (const TokenSpan& span : line_highlight.spans) {
        U8String token_text = extractScopeTokenText(line_text, span);
        if (token_text.empty()) {
          continue;
        }
        if (shouldSkipScopeToken(rule, span)) {
          continue;
        }

        auto handleIndentStartToken = [&](const U8String& token) {
          for (const ScopeRule* br : indent_scope_rules) {
            if (token == br->start) {
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
              return true;
            }
          }
          return false;
        };

        matched_start = handleIndentStartToken(token_text);
        if (!matched_start) {
          size_t token_char_count = Utf8Util::countChars(token_text);
          if (token_char_count > 1) {
            for (size_t i = 0; i < token_char_count; ++i) {
              U8String one_char = Utf8Util::utf8Substr(token_text, i, 1);
              if (one_char.empty()) {
                continue;
              }
              if (handleIndentStartToken(one_char)) {
                matched_start = true;
                break;
              }
            }
          }
        }

        if (matched_start) {
          break;
        }
      }
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
