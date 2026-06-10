#include <algorithm>

#include "internal_highlight.h"
#include "sweetline/util.h"

namespace NS_SWEETLINE {
  namespace {
    bool matchesAt(const U8String& text, size_t byte_pos, const U8String& token) {
      return !token.empty()
        && byte_pos + token.size() <= text.size()
        && text.compare(byte_pos, token.size(), token) == 0;
    }

    size_t findSkipEnd(const U8String& text, size_t byte_pos, const ScopeSkipRule& rule) {
      size_t pos = byte_pos;
      while (pos < text.size()) {
        if (!rule.escape.empty() && matchesAt(text, pos, rule.escape)) {
          pos += rule.escape.size();
          if (pos < text.size()) {
            pos += 1;
          }
          continue;
        }
        if (matchesAt(text, pos, rule.end)) {
          return pos + rule.end.size();
        }
        pos++;
      }
      return U8String::npos;
    }

    int32_t toColumn(const U8String& text, size_t byte_pos) {
      return static_cast<int32_t>(Utf8Util::bytePosToCharPos(text, byte_pos));
    }

    int32_t tokenLength(const U8String& token) {
      return static_cast<int32_t>(Utf8Util::countChars(token));
    }

    LineRange normalizeLineRange(const SharedPtr<Document>& document, const LineRange& range) {
      if (document == nullptr || document->getLineCount() == 0 || range.line_count == 0) {
        return {0, 0};
      }
      size_t start = std::min(range.start_line, document->getLineCount());
      if (start >= document->getLineCount()) {
        return {document->getLineCount(), 0};
      }
      size_t available = document->getLineCount() - start;
      return {start, std::min(range.line_count, available)};
    }

    List<const ScopeSkipRule*> collectSkipRulesOrdered(const SharedPtr<SyntaxRule>& rule) {
      List<const ScopeSkipRule*> ordered_rules;
      if (rule == nullptr) {
        return ordered_rules;
      }
      ordered_rules.reserve(rule->bracket_skip_rules.size());
      for (const ScopeSkipRule& skip_rule : rule->bracket_skip_rules) {
        ordered_rules.push_back(&skip_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const ScopeSkipRule* lhs, const ScopeSkipRule* rhs) {
          return lhs->start.size() > rhs->start.size();
        });
      return ordered_rules;
    }

    List<const BracketRule*> collectOpenBracketRulesOrdered(const SharedPtr<SyntaxRule>& rule) {
      List<const BracketRule*> ordered_rules;
      if (rule == nullptr) {
        return ordered_rules;
      }
      ordered_rules.reserve(rule->bracket_rules.size());
      for (const BracketRule& bracket_rule : rule->bracket_rules) {
        ordered_rules.push_back(&bracket_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const BracketRule* lhs, const BracketRule* rhs) {
          return lhs->start.size() > rhs->start.size();
        });
      return ordered_rules;
    }

    List<const BracketRule*> collectCloseBracketRulesOrdered(const SharedPtr<SyntaxRule>& rule) {
      List<const BracketRule*> ordered_rules;
      if (rule == nullptr) {
        return ordered_rules;
      }
      ordered_rules.reserve(rule->bracket_rules.size());
      for (const BracketRule& bracket_rule : rule->bracket_rules) {
        ordered_rules.push_back(&bracket_rule);
      }
      std::sort(ordered_rules.begin(), ordered_rules.end(),
        [](const BracketRule* lhs, const BracketRule* rhs) {
          return lhs->end.size() > rhs->end.size();
        });
      return ordered_rules;
    }

    TextRange makeRange(size_t line, int32_t column, int32_t length, size_t line_start_index) {
      TextRange range;
      range.start = {line, static_cast<size_t>(column), line_start_index + static_cast<size_t>(column)};
      range.end = {line, static_cast<size_t>(column + length), line_start_index + static_cast<size_t>(column + length)};
      return range;
    }

    TextRange emptyPartnerRange() {
      return {};
    }
  }

  BracketPairAnalyzer::BracketPairAnalyzer(const SharedPtr<SyntaxRule>& rule,
    const SharedPtr<Document>& document,
    const HighlightConfig& config)
    : m_rule_(rule),
      m_document_(document),
      m_config_(config),
      m_ordered_skip_rules_(collectSkipRulesOrdered(rule)),
      m_ordered_open_bracket_rules_(collectOpenBracketRulesOrdered(rule)),
      m_ordered_close_bracket_rules_(collectCloseBracketRulesOrdered(rule)) {
  }

  SharedPtr<BracketPairResult> BracketPairAnalyzer::analyzeLineRange(const LineRange& visible_range) {
    auto result = makeSharedPtr<BracketPairResult>();
    if (m_document_ == nullptr) {
      return result;
    }
    result->total_line_count = m_document_->getLineCount();
    LineRange normalized_range = normalizeLineRange(m_document_, visible_range);
    result->start_line = normalized_range.start_line;
    result->lines.resize(normalized_range.line_count);
    if (normalized_range.line_count == 0 || m_ordered_open_bracket_rules_.empty()) {
      return result;
    }

    const size_t total_line_count = m_document_->getLineCount();
    const size_t visible_start = normalized_range.start_line;
    const size_t visible_end = visible_start + normalized_range.line_count - 1;
    const size_t scan_end = std::min(total_line_count - 1, visible_end + m_lookahead_lines_);

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

    BracketMatchState unresolved_state = scan_end + 1 >= total_line_count
      ? BracketMatchState::UNMATCHED
      : BracketMatchState::UNKNOWN;
    for (ActiveBracket& bracket : state.brackets) {
      if (bracket.result_line_index >= 0 && bracket.result_token_index >= 0) {
        BracketToken& token = result->lines[static_cast<size_t>(bracket.result_line_index)]
          .tokens[static_cast<size_t>(bracket.result_token_index)];
        if (token.match_state == BracketMatchState::UNKNOWN) {
          token.match_state = unresolved_state;
        }
      }
    }
    return result;
  }

  void BracketPairAnalyzer::invalidateFrom(size_t line) {
    if (m_checkpoints_.empty()) {
      return;
    }
    auto it = std::remove_if(m_checkpoints_.begin(), m_checkpoints_.end(),
      [line](const Checkpoint& checkpoint) {
        return checkpoint.line >= line;
      });
    m_checkpoints_.erase(it, m_checkpoints_.end());
  }

  void BracketPairAnalyzer::reset() {
    m_checkpoints_.clear();
  }

  BracketPairAnalyzer::ScanState BracketPairAnalyzer::getStateForLine(size_t line) {
    ScanState state;
    size_t scan_start = 0;
    for (const Checkpoint& checkpoint : m_checkpoints_) {
      if (checkpoint.line <= line && checkpoint.line >= scan_start) {
        scan_start = checkpoint.line;
        state = checkpoint.state;
      }
    }
    for (size_t scan_line = scan_start; scan_line < line; ++scan_line) {
      scanLine(scan_line, state, nullptr);
      if (scan_line + 1 < line && (scan_line + 1) % m_checkpoint_interval_ == 0) {
        saveCheckpoint(scan_line + 1, state);
      }
    }
    return state;
  }

  void BracketPairAnalyzer::saveCheckpoint(size_t line, const ScanState& state) {
    Checkpoint checkpoint;
    checkpoint.line = line;
    checkpoint.state = state;
    for (ActiveBracket& bracket : checkpoint.state.brackets) {
      bracket.result_line_index = -1;
      bracket.result_token_index = -1;
    }
    auto it = std::lower_bound(m_checkpoints_.begin(), m_checkpoints_.end(), line,
      [](const Checkpoint& checkpoint, size_t target_line) {
        return checkpoint.line < target_line;
      });
    if (it != m_checkpoints_.end() && it->line == line) {
      *it = std::move(checkpoint);
    } else {
      m_checkpoints_.insert(it, std::move(checkpoint));
    }
  }

  void BracketPairAnalyzer::addVisibleToken(const BracketToken& token, ScanContext* context, BracketToken*& result_token) const {
    result_token = nullptr;
    if (context == nullptr || context->result == nullptr) {
      return;
    }
    if (token.range.start.line < context->visible_start || token.range.start.line > context->visible_end) {
      return;
    }
    size_t line_index = token.range.start.line - context->visible_start;
    LineBracketPairs& line_result = context->result->lines[line_index];
    line_result.tokens.push_back(token);
    result_token = &line_result.tokens.back();
  }

  void BracketPairAnalyzer::scanLine(size_t line, ScanState& state, ScanContext* context) {
    if (m_document_ == nullptr || line >= m_document_->getLineCount()) {
      return;
    }
    const U8String& text = m_document_->getLine(line).text;
    const size_t line_start_index = m_document_->charIndexOfLine(line);
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
      for (const BracketRule* bracket_rule : m_ordered_close_bracket_rules_) {
        if (!matchesAt(text, byte_pos, bracket_rule->end)) {
          continue;
        }
        int32_t match_index = -1;
        for (int32_t i = static_cast<int32_t>(state.brackets.size()) - 1; i >= 0; --i) {
          if (state.brackets[static_cast<size_t>(i)].rule == bracket_rule) {
            match_index = i;
            break;
          }
        }
        const int32_t column = toColumn(text, byte_pos);
        const int32_t length = tokenLength(bracket_rule->end);
        BracketToken close_token;
        close_token.range = makeRange(line, column, length, line_start_index);
        close_token.kind = BracketTokenKind::CLOSE;

        if (match_index < 0) {
          close_token.depth = static_cast<int32_t>(state.brackets.size());
          close_token.match_state = BracketMatchState::UNMATCHED;
          BracketToken* ignored = nullptr;
          addVisibleToken(close_token, context, ignored);
        } else {
          for (size_t i = state.brackets.size(); i-- > static_cast<size_t>(match_index + 1);) {
            ActiveBracket& unmatched = state.brackets[i];
            if (unmatched.result_line_index >= 0 && unmatched.result_token_index >= 0 && context != nullptr) {
              BracketToken& token = context->result->lines[static_cast<size_t>(unmatched.result_line_index)]
                .tokens[static_cast<size_t>(unmatched.result_token_index)];
              token.match_state = BracketMatchState::UNMATCHED;
            }
          }

          ActiveBracket& open_bracket = state.brackets[static_cast<size_t>(match_index)];
          close_token.depth = open_bracket.depth;
          close_token.match_state = BracketMatchState::MATCHED;
          close_token.partner_range = open_bracket.token.range;
          if (open_bracket.result_line_index >= 0 && open_bracket.result_token_index >= 0 && context != nullptr) {
            BracketToken& open_token = context->result->lines[static_cast<size_t>(open_bracket.result_line_index)]
              .tokens[static_cast<size_t>(open_bracket.result_token_index)];
            open_token.match_state = BracketMatchState::MATCHED;
            open_token.partner_range = close_token.range;
          }
          BracketToken* ignored = nullptr;
          addVisibleToken(close_token, context, ignored);
          state.brackets.resize(static_cast<size_t>(match_index));
        }

        byte_pos += bracket_rule->end.size();
        handled = true;
        break;
      }
      if (handled) {
        continue;
      }

      for (const BracketRule* bracket_rule : m_ordered_open_bracket_rules_) {
        if (!matchesAt(text, byte_pos, bracket_rule->start)) {
          continue;
        }
        const int32_t column = toColumn(text, byte_pos);
        const int32_t length = tokenLength(bracket_rule->start);
        BracketToken token;
        token.range = makeRange(line, column, length, line_start_index);
        token.depth = static_cast<int32_t>(state.brackets.size());
        token.kind = BracketTokenKind::OPEN;
        token.match_state = BracketMatchState::UNKNOWN;
        token.partner_range = emptyPartnerRange();

        BracketToken* result_token = nullptr;
        addVisibleToken(token, context, result_token);

        ActiveBracket active;
        active.rule = bracket_rule;
        active.depth = token.depth;
        active.token = token;
        if (result_token != nullptr && context != nullptr) {
          active.result_line_index = static_cast<int32_t>(token.range.start.line - context->visible_start);
          active.result_token_index = static_cast<int32_t>(
            context->result->lines[static_cast<size_t>(active.result_line_index)].tokens.size() - 1);
        }
        state.brackets.push_back(std::move(active));
        byte_pos += bracket_rule->start.size();
        handled = true;
        break;
      }
      if (handled) {
        continue;
      }

      byte_pos++;
    }
  }
}
