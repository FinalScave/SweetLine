#ifndef SWEETLINE_INTERNAL_SYNTAX_H
#define SWEETLINE_INTERNAL_SYNTAX_H

#include <cstdint>
#include <oniguruma/oniguruma.h>
#include <nlohmann/json.hpp>
#include "syntax.h"

namespace NS_SWEETLINE {
  class HighlightEngine;
}

namespace NS_SWEETLINE {
  /// Minimum unit (token) rule for match analysis
  struct TokenRule {
    /// Regex pattern
    U8String pattern;
    /// Style IDs by capture group (used when inlineStyle is not specified)
    HashMap<int32_t, int32_t> style_ids;
    /// Goto state string parsed from JSON
    U8String goto_state_str;
    /// Number of regex capture groups in this token
    int32_t group_count {0};
    /// Group offset start of this token's capture groups in the merged expression
    int32_t group_offset_start {0};
    /// Target state to transition to
    int32_t goto_state {-1};

    /// SubState name by capture group
    HashMap<int32_t, U8String> sub_state_strs;
    /// SubState ID by capture group
    HashMap<int32_t, int32_t> sub_states;

    int32_t getGroupStyleId(int32_t group) const;
    int32_t getGroupSubState(int32_t group) const;

    static int32_t kDefaultStyleId;
    static TokenRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// importSyntax request (recorded during parsing, processed after compilation)
  struct ImportSyntaxRequest {
    /// Name of the syntax rule to import
    U8String syntax_name;
    /// #ifdef macro condition (empty means unconditional import)
    U8String ifdef_macro;
  };

  /// Rule for each state
  struct StateRule {
    /// State name
    U8String name;
    /// Token rules for each token
    List<TokenRule> token_rules;
    /// State name to transition to at line end
    U8String line_end_state_str;
    /// State to transition to at line end
    int32_t line_end_state {-1};
    /// Merged pattern combining all token patterns
    U8String merged_pattern;
    /// Compiled regex pointer
    OnigRegex regex;
    /// Total capture group count of the merged pattern
    int32_t group_count {0};
    /// importSyntax request list
    List<ImportSyntaxRequest> import_requests;

    static StateRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// Scope rule for indent guides (defining scope start/end markers and branch keywords)
  struct ScopeRule {
    /// Scope start text (e.g. {, @begin)
    U8String start;
    /// Scope end text (e.g. }, @end)
    U8String end;
    /// Branch keywords
    std::unordered_set<U8String> branch_keywords;
    /// ID, generated after parsing
    int32_t rule_id {0};

#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// Syntax rule compiler
  class SyntaxRuleCompiler {
  public:
    explicit SyntaxRuleCompiler(const SharedPtr<StyleMapping>& style_mapping, bool inline_style, HighlightEngine* engine = nullptr);

    /// Compile syntax rule from JSON
    /// @param json JSON content of the syntax rule
    SharedPtr<SyntaxRule> compileSyntaxFromJson(const U8String& json);

    /// Compile syntax rule from file
    /// @param file Syntax rule definition file (JSON)
    SharedPtr<SyntaxRule> compileSyntaxFromFile(const U8String& file);
  private:
    SharedPtr<StyleMapping> m_style_mapping_;
    bool m_inline_style_ {false};
    HighlightEngine* m_engine_ {nullptr};

    static void parseSyntaxName(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseFileExtensions(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseVariables(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseInlineStyles(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    void parseStates(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    void parseState(const SharedPtr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json);
		void parseScopeRules(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void compileStatePattern(StateRule& state_rule);
    void processImportSyntaxRequests(const SharedPtr<SyntaxRule>& rule);
    void importSyntaxRule(const SharedPtr<SyntaxRule>& target_rule, int32_t target_state_id,
      const SharedPtr<SyntaxRule>& source_rule);
    static void replaceVariable(U8String& text, HashMap<U8String, U8String>& variables_map);

    static int32_t parseColorSafe(const U8String& color_str);
  };
}

#endif //SWEETLINE_INTERNAL_SYNTAX_H