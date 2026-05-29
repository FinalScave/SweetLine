#include <algorithm>

#include "internal_highlight.h"
#include "sweetline/util.h"

namespace NS_SWEETLINE {
  namespace {
    bool isAsciiWordChar(char ch) {
      return (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= '0' && ch <= '9')
        || ch == '_';
    }

    bool isWordToken(const U8String& token) {
      if (token.empty()) {
        return false;
      }
      for (char ch : token) {
        if (!isAsciiWordChar(ch)) {
          return false;
        }
      }
      return true;
    }

    bool hasWordBoundary(const U8String& text, size_t byte_pos, const U8String& token) {
      if (byte_pos > 0 && isAsciiWordChar(text[byte_pos - 1])) {
        return false;
      }
      size_t end = byte_pos + token.size();
      return end >= text.size() || !isAsciiWordChar(text[end]);
    }

    bool matchesAt(const U8String& text, size_t byte_pos, const U8String& token) {
      return !token.empty()
        && byte_pos + token.size() <= text.size()
        && text.compare(byte_pos, token.size(), token) == 0;
    }

    bool matchesRuleToken(const U8String& text, size_t byte_pos, const U8String& token, ScopeRuleKind kind) {
      if (!matchesAt(text, byte_pos, token)) {
        return false;
      }
      return kind != ScopeRuleKind::WORD || hasWordBoundary(text, byte_pos, token);
    }

    bool matchesBranchToken(const U8String& text, size_t byte_pos, const U8String& token) {
      if (!matchesAt(text, byte_pos, token)) {
        return false;
      }
      return !isWordToken(token) || hasWordBoundary(text, byte_pos, token);
    }

    bool isBlankLine(const U8String& text) {
      return text.empty() || text.find_first_not_of(" \t") == U8String::npos;
    }

    int32_t toColumn(const U8String& text, size_t byte_pos) {
      return static_cast<int32_t>(Utf8Util::bytePosToCharPos(text, byte_pos));
    }

    size_t findSkipEnd(const U8String& text, size_t byte_pos, const ScopeSkipRule& rule) {
      size_t pos = byte_pos;
      while (pos < text.size()) {
        if (!rule.escape.empty() && matchesAt(text, pos, rule.escape)) {
          pos += rule.escape.size();
          if (pos < text.size()) {
            ++pos;
          }
          continue;
        }
        if (matchesAt(text, pos, rule.end)) {
          return pos + rule.end.size();
        }
        ++pos;
      }
      return U8String::npos;
    }

    List<const ScopeSkipRule*> collectSkipRulesOrdered(const SharedPtr<SyntaxRule>& rule) {
      List<const ScopeSkipRule*> ordered_rules;
      if (rule == nullptr) {
        return ordered_rules;
      }
      ordered_rules.reserve(rule->scope_skip_rules.size());
      for (const ScopeSkipRule& skip_rule : rule->scope_skip_rules) {
        ordered_rules.push_back(&skip_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const ScopeSkipRule* lhs, const ScopeSkipRule* rhs) {
          if (lhs->start.size() != rhs->start.size()) {
            return lhs->start.size() > rhs->start.size();
          }
          return lhs->rule_id < rhs->rule_id;
        });
      return ordered_rules;
    }

    List<const ScopeRule*> collectScopeRulesOrdered(const SharedPtr<SyntaxRule>& rule) {
      List<const ScopeRule*> ordered_rules;
      if (rule == nullptr) {
        return ordered_rules;
      }
      ordered_rules.reserve(rule->scope_rules.size());
      for (const ScopeRule& scope_rule : rule->scope_rules) {
        ordered_rules.push_back(&scope_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const ScopeRule* lhs, const ScopeRule* rhs) {
          if (lhs->start.size() != rhs->start.size()) {
            return lhs->start.size() > rhs->start.size();
          }
          return lhs->rule_id < rhs->rule_id;
        });
      return ordered_rules;
    }

    LineRange normalizeLineRange(const SharedPtr<Document>& document, const LineRange& visible_range) {
      LineRange result;
      if (document == nullptr) {
        return result;
      }
      size_t total_line_count = document->getLineCount();
      result.start_line = std::min(visible_range.start_line, total_line_count);
      if (visible_range.line_count == 0 || result.start_line >= total_line_count) {
        result.line_count = 0;
        return result;
      }
      result.line_count = std::min(visible_range.line_count, total_line_count - result.start_line);
      return result;
    }
  }

  ScopeGuideAnalyzer::ScopeGuideAnalyzer(const SharedPtr<SyntaxRule>& rule,
    const SharedPtr<Document>& document, const HighlightConfig& config)
    : m_rule_(rule), m_document_(document), m_config_(config),
      m_ordered_skip_rules_(collectSkipRulesOrdered(rule)),
      m_ordered_scope_rules_(collectScopeRulesOrdered(rule)) {
    reset();
  }

  int32_t ScopeGuideAnalyzer::computeLeadingWhitespace(const U8String& text, int32_t tab_size) {
    int32_t columns = 0;
    for (char ch : text) {
      if (ch == ' ') {
        ++columns;
      } else if (ch == '\t') {
        columns += tab_size - (columns % tab_size);
      } else {
        break;
      }
    }
    return columns;
  }

  void ScopeGuideAnalyzer::reset() {
    m_checkpoints_.clear();
    m_checkpoints_.push_back({});
  }

  void ScopeGuideAnalyzer::invalidateFrom(size_t line) {
    if (m_checkpoints_.empty()) {
      reset();
      return;
    }
    m_checkpoints_.erase(
      std::remove_if(m_checkpoints_.begin(), m_checkpoints_.end(),
        [line](const Checkpoint& checkpoint) {
          return checkpoint.line > line;
        }),
      m_checkpoints_.end());
    if (m_checkpoints_.empty()) {
      reset();
    }
  }

  SharedPtr<IndentGuideResult> ScopeGuideAnalyzer::analyzeLineRange(const LineRange& visible_range) {
    auto result = makeSharedPtr<IndentGuideResult>();
    if (m_document_ == nullptr || m_config_.tab_size <= 0) {
      return result;
    }
    result->total_line_count = m_document_->getLineCount();
    LineRange normalized_range = normalizeLineRange(m_document_, visible_range);
    result->start_line = normalized_range.start_line;
    if (normalized_range.line_count == 0) {
      return result;
    }
    analyzeByScopeRules(normalized_range, result);
    return result;
  }

  void ScopeGuideAnalyzer::analyzeByScopeRules(const LineRange& visible_range,
    SharedPtr<IndentGuideResult>& result) {
    if (m_document_ == nullptr || result == nullptr || visible_range.line_count == 0) {
      return;
    }
    const size_t total_line_count = m_document_->getLineCount();
    const size_t visible_start = visible_range.start_line;
    const size_t visible_end = visible_start + visible_range.line_count - 1;
    const size_t scan_end = std::min(total_line_count - 1, visible_end + m_lookahead_lines_);

    result->line_states.resize(visible_range.line_count);

    ScanState state = getStateForLine(visible_start);
    ScanContext context;
    context.result = result;
    context.visible_start = visible_start;
    context.visible_end = visible_end;

    for (size_t line = visible_start; line <= scan_end; ++line) {
      if (line != visible_start && line % m_checkpoint_interval_ == 0) {
        saveCheckpoint(line, state);
      }
      scanLine(line, state, &context);
    }

    for (ActiveScope& scope : state.scopes) {
      if (scope.guide_index < 0) {
        continue;
      }
      IndentGuideLine& guide = result->guide_lines[scope.guide_index];
      guide.end_line = static_cast<int32_t>(visible_end);
      guide.continues_after = visible_end + 1 < total_line_count;
    }
  }

  ScopeGuideAnalyzer::ScanState ScopeGuideAnalyzer::getStateForLine(size_t line) {
    ScanState state;
    size_t start_line = 0;
    for (const Checkpoint& checkpoint : m_checkpoints_) {
      if (checkpoint.line <= line && checkpoint.line >= start_line) {
        state = checkpoint.state;
        start_line = checkpoint.line;
      }
    }
    if (m_document_ == nullptr) {
      return state;
    }
    size_t total_line_count = m_document_->getLineCount();
    size_t target_line = std::min(line, total_line_count);
    for (size_t current_line = start_line; current_line < target_line; ++current_line) {
      if (current_line != start_line && current_line % m_checkpoint_interval_ == 0) {
        saveCheckpoint(current_line, state);
      }
      scanLine(current_line, state, nullptr);
    }
    saveCheckpoint(target_line, state);
    return state;
  }

  void ScopeGuideAnalyzer::saveCheckpoint(size_t line, const ScanState& state) {
    ScanState checkpoint_state = state;
    for (ActiveScope& scope : checkpoint_state.scopes) {
      scope.guide_index = -1;
    }
    auto existing = std::find_if(m_checkpoints_.begin(), m_checkpoints_.end(),
      [line](const Checkpoint& checkpoint) {
        return checkpoint.line == line;
      });
    if (existing != m_checkpoints_.end()) {
      existing->state = std::move(checkpoint_state);
      return;
    }
    Checkpoint checkpoint;
    checkpoint.line = line;
    checkpoint.state = std::move(checkpoint_state);
    m_checkpoints_.push_back(std::move(checkpoint));
    std::sort(m_checkpoints_.begin(), m_checkpoints_.end(),
      [](const Checkpoint& lhs, const Checkpoint& rhs) {
        return lhs.line < rhs.line;
      });
  }

  void ScopeGuideAnalyzer::scanLine(size_t line, ScanState& state, ScanContext* context) {
    if (m_document_ == nullptr || line >= m_document_->getLineCount()) {
      return;
    }

    const U8String& text = m_document_->getLine(line).text;
    const bool blank_line = isBlankLine(text);
    const int32_t indent_column = blank_line ? -1 : computeLeadingWhitespace(text, m_config_.tab_size);
    const bool visible = context != nullptr && line >= context->visible_start && line <= context->visible_end;
    const size_t visible_index = visible ? line - context->visible_start : 0;
    const bool use_indentation_only = m_rule_ == nullptr || m_rule_->scope_rules.empty();

    auto openGuide = [&](ActiveScope& scope, int32_t nesting_level, bool continues_before) {
      if (context == nullptr || scope.guide_index >= 0) {
        return;
      }
      IndentGuideLine guide;
      guide.column = scope.guide_column;
      guide.start_line = continues_before
        ? static_cast<int32_t>(context->visible_start)
        : std::max(scope.start_line, static_cast<int32_t>(context->visible_start));
      guide.end_line = guide.start_line;
      guide.nesting_level = nesting_level;
      guide.scope_rule_id = scope.rule == nullptr ? -1 : scope.rule->rule_id;
      guide.continues_before = continues_before;
      for (const IndentGuideLine::BranchPoint& branch : scope.branches) {
        if (branch.line >= static_cast<int32_t>(context->visible_start)
          && branch.line <= static_cast<int32_t>(context->visible_end)) {
          guide.branches.push_back(branch);
        }
      }
      scope.guide_index = static_cast<int32_t>(context->result->guide_lines.size());
      context->result->guide_lines.push_back(std::move(guide));
    };

    auto openVisibleGuides = [&]() {
      if (context == nullptr) {
        return;
      }
      for (size_t i = 0; i < state.scopes.size(); ++i) {
        ActiveScope& scope = state.scopes[i];
        if (scope.start_line > static_cast<int32_t>(context->visible_end)) {
          continue;
        }
        openGuide(scope, static_cast<int32_t>(i), scope.start_line < static_cast<int32_t>(context->visible_start));
      }
    };

    auto closeScope = [&](size_t scope_index, int32_t end_line, int32_t end_column, bool explicit_end) {
      if (scope_index >= state.scopes.size()) {
        return;
      }
      ActiveScope scope = std::move(state.scopes[scope_index]);
      if (explicit_end) {
        scope.guide_column = std::min(scope.guide_column, end_column);
      }
      if (scope.guide_index >= 0 && context != nullptr) {
        IndentGuideLine& guide = context->result->guide_lines[scope.guide_index];
        guide.column = scope.guide_column;
        guide.end_line = std::min(end_line, static_cast<int32_t>(context->visible_end));
        guide.continues_after = end_line > static_cast<int32_t>(context->visible_end);
        if (guide.end_line < guide.start_line) {
          guide.end_line = guide.start_line;
        }
      }
      state.scopes.erase(state.scopes.begin() + static_cast<ptrdiff_t>(scope_index));
    };

    if (!blank_line) {
      for (int32_t i = static_cast<int32_t>(state.scopes.size()) - 1; i >= 0; --i) {
        ActiveScope& scope = state.scopes[static_cast<size_t>(i)];
        bool should_close_indent = scope.kind == ScopeRuleKind::INDENT_START
          && scope.rule != nullptr
          && indent_column <= scope.start_indent;
        bool should_close_pure_indent = use_indentation_only
          && scope.rule == nullptr
          && scope.guide_column > indent_column;
        if (!should_close_indent && !should_close_pure_indent) {
          continue;
        }
        int32_t end_line = line == 0 ? 0 : static_cast<int32_t>(line - 1);
        closeScope(static_cast<size_t>(i), end_line, scope.guide_column, false);
      }
    }

    if (visible) {
      LineScopeState& line_state = context->result->line_states[visible_index];
      line_state.indent_level = blank_line ? static_cast<int32_t>(state.scopes.size()) : indent_column / m_config_.tab_size;
      line_state.nesting_level = static_cast<int32_t>(state.scopes.size());
      line_state.scope_state = ScopeState::CONTENT;
      line_state.scope_column = 0;
    }

    if (context != nullptr && line == context->visible_start) {
      openVisibleGuides();
    }

    if (use_indentation_only) {
      if (!blank_line) {
        int32_t next_column = m_config_.tab_size;
        if (!state.scopes.empty()) {
          next_column = state.scopes.back().guide_column + m_config_.tab_size;
        }
        while (next_column <= indent_column) {
          ActiveScope scope;
          scope.kind = ScopeRuleKind::INDENT_START;
          scope.start_line = static_cast<int32_t>(line);
          scope.start_indent = next_column - m_config_.tab_size;
          scope.guide_column = next_column;
          state.scopes.push_back(std::move(scope));
          if (visible) {
            openGuide(state.scopes.back(), static_cast<int32_t>(state.scopes.size()) - 1, false);
          }
          next_column += m_config_.tab_size;
        }
        if (visible) {
          LineScopeState& line_state = context->result->line_states[visible_index];
          line_state.nesting_level = static_cast<int32_t>(state.scopes.size());
        }
      }
      return;
    }

    size_t byte_pos = 0;
    while (byte_pos < text.size()) {
      if (state.skip.active && state.skip.rule != nullptr) {
        size_t end_pos = findSkipEnd(text, byte_pos, *state.skip.rule);
        if (end_pos == U8String::npos) {
          if (!state.skip.rule->multi_line) {
            state.skip = {};
          }
          return;
        }
        byte_pos = end_pos;
        state.skip = {};
        continue;
      }

      bool skipped = false;
      for (const ScopeSkipRule* skip_rule : m_ordered_skip_rules_) {
        if (!matchesAt(text, byte_pos, skip_rule->start)) {
          continue;
        }
        if (skip_rule->kind == ScopeSkipKind::LINE_COMMENT) {
          return;
        }
        size_t skip_start_end = byte_pos + skip_rule->start.size();
        size_t end_pos = findSkipEnd(text, skip_start_end, *skip_rule);
        if (end_pos == U8String::npos) {
          if (skip_rule->multi_line) {
            state.skip.active = true;
            state.skip.rule = skip_rule;
          }
          return;
        }
        byte_pos = end_pos;
        skipped = true;
        break;
      }
      if (skipped) {
        continue;
      }

      bool handled = false;
      for (int32_t i = static_cast<int32_t>(state.scopes.size()) - 1; i >= 0; --i) {
        ActiveScope& scope = state.scopes[static_cast<size_t>(i)];
        if (scope.rule == nullptr || scope.rule->end.empty()) {
          continue;
        }
        if (!matchesRuleToken(text, byte_pos, scope.rule->end, scope.kind)) {
          continue;
        }
        const int32_t token_column = toColumn(text, byte_pos);
        const size_t token_size = scope.rule->end.size();
        if (visible) {
          LineScopeState& line_state = context->result->line_states[visible_index];
          line_state.scope_state = ScopeState::END;
          line_state.nesting_level = i;
          line_state.scope_column = std::min(scope.guide_column, token_column);
        }
        closeScope(static_cast<size_t>(i), static_cast<int32_t>(line), token_column, true);
        byte_pos += token_size;
        handled = true;
        break;
      }
      if (handled) {
        continue;
      }

      for (int32_t i = static_cast<int32_t>(state.scopes.size()) - 1; i >= 0; --i) {
        ActiveScope& scope = state.scopes[static_cast<size_t>(i)];
        if (scope.rule == nullptr) {
          continue;
        }
        for (const U8String& branch : scope.rule->branch_keywords) {
          if (!matchesBranchToken(text, byte_pos, branch)) {
            continue;
          }
          const int32_t token_column = toColumn(text, byte_pos);
          IndentGuideLine::BranchPoint branch_point {static_cast<int32_t>(line), token_column};
          scope.branches.push_back(branch_point);
          if (visible && scope.guide_index >= 0) {
            context->result->guide_lines[scope.guide_index].branches.push_back(branch_point);
          }
          byte_pos += branch.size();
          handled = true;
          break;
        }
        if (handled) {
          break;
        }
      }
      if (handled) {
        continue;
      }

      for (const ScopeRule* scope_rule : m_ordered_scope_rules_) {
        if (!matchesRuleToken(text, byte_pos, scope_rule->start, scope_rule->kind)) {
          continue;
        }
        const int32_t token_column = toColumn(text, byte_pos);
        ActiveScope scope;
        scope.rule = scope_rule;
        scope.kind = scope_rule->kind;
        scope.start_line = static_cast<int32_t>(line);
        scope.start_indent = blank_line ? 0 : indent_column;
        scope.guide_column = scope_rule->kind == ScopeRuleKind::INDENT_START
          ? scope.start_indent + m_config_.tab_size
          : token_column;
        state.scopes.push_back(std::move(scope));
        if (visible) {
          ActiveScope& active_scope = state.scopes.back();
          openGuide(active_scope, static_cast<int32_t>(state.scopes.size()) - 1, false);
          LineScopeState& line_state = context->result->line_states[visible_index];
          line_state.scope_state = ScopeState::START;
          line_state.nesting_level = static_cast<int32_t>(state.scopes.size()) - 1;
          line_state.scope_column = scope_rule->kind == ScopeRuleKind::INDENT_START
            ? active_scope.start_indent
            : token_column;
        }
        byte_pos += scope_rule->start.size();
        handled = true;
        break;
      }
      if (handled) {
        continue;
      }

      ++byte_pos;
    }
  }
}
