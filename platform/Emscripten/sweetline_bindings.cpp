#include <emscripten/bind.h>
#include <unordered_set>
#include "foundation.h"
#include "highlight.h"
#include "internal_highlight.h"

using namespace NS_SWEETLINE;

#define BIND_LIST(element_type, name) \
  emscripten::class_<List<element_type>>(name) \
    .constructor<>() \
    .function("get", emscripten::select_overload<element_type&(const size_t)>(&List<element_type>::at)) \
    .function("set", emscripten::optional_override([](List<element_type>& self, size_t index, const element_type& element) { \
        self[index] = element; \
      }) \
    ) \
    .function("add", emscripten::optional_override([](List<element_type>& self, const element_type& element) { \
        self.push_back(element); \
      }) \
    ) \
    .function("remove", emscripten::optional_override([](List<element_type>& self, const element_type& element) { \
        self.erase(std::remove(self.begin(), self.end(), element), self.end());; \
      }) \
    ) \
    .function("isEmpty", &List<element_type>::empty) \
    .function("size", &List<element_type>::size);

extern "C" {

EMSCRIPTEN_BINDINGS(foundation) {
  emscripten::class_<TextPosition>("TextPosition")
    .constructor<>()
    .property("line", &TextPosition::line)
    .property("column", &TextPosition::column)
    .property("index", &TextPosition::index);

  emscripten::class_<TextRange>("TextRange")
    .constructor<>()
    .property("start", &TextRange::start)
    .property("end", &TextRange::end);

  emscripten::class_<Document>("Document")
    .smart_ptr<SharedPtr<Document>>("SharedPtr<Document>")
    .constructor<>(emscripten::optional_override([](const U8String& uri, const U8String& content) {
        return makeSharedPtr<Document>(uri, content);
      })
    )
    .function("getUri", &Document::getUri)
    .function("getText", &Document::getText)
    .function("totalChars", &Document::totalChars)
    .function("getLineCharCount", &Document::getLineCharCount)
    .function("getLineCount", &Document::getLineCount)
    .function("getLine", &Document::getLine)
    .function("charIndexOfLine", &Document::charIndexOfLine)
    .function("charIndexToPosition", &Document::charIndexToPosition);
}

EMSCRIPTEN_BINDINGS(highlight) {
  emscripten::class_<InlineStyle>("InlineStyle")
    .constructor<>()
    .property("foreground", &InlineStyle::foreground)
    .property("background", &InlineStyle::background)
    .property("isBold", &InlineStyle::is_bold)
    .property("isItalic", &InlineStyle::is_italic)
    .property("isStrikethrough", &InlineStyle::is_strikethrough);
  emscripten::class_<TokenSpan>("TokenSpan")
    .constructor<>()
    .property("range", &TokenSpan::range)
    .property("styleId", &TokenSpan::style_id)
    .property("inlineStyle", &TokenSpan::inline_style);
  BIND_LIST(TokenSpan, "TokenSpanList")

  emscripten::class_<LineHighlight>("LineHighlight")
    .constructor<>()
    .property("spans", &LineHighlight::spans)
    .function("toJson",emscripten::optional_override([](const SharedPtr<LineHighlight>& self) {
        U8String json;
        self->toJson(json);
        return json;
      })
    );
  BIND_LIST(LineHighlight, "LineHighlightList")

  emscripten::class_<DocumentHighlight>("DocumentHighlight")
    .smart_ptr<SharedPtr<DocumentHighlight>>("SharedPtr<DocumentHighlight>")
    .property("lines", &DocumentHighlight::lines)
    .function("toJson",emscripten::optional_override([](const SharedPtr<DocumentHighlight>& self) {
        U8String json;
        self->toJson(json);
        return json;
      })
    );

  emscripten::class_<TextLineInfo>("TextLineInfo")
    .constructor<>()
    .property("line", &TextLineInfo::line)
    .property("startState", &TextLineInfo::start_state)
    .property("startCharOffset", &TextLineInfo::start_char_offset);

  emscripten::class_<LineAnalyzeResult>("LineAnalyzeResult")
    .constructor<>()
    .property("highlight", &LineAnalyzeResult::highlight)
    .property("endState", &LineAnalyzeResult::end_state)
    .property("charCount", &LineAnalyzeResult::char_count);

  emscripten::class_<TextAnalyzer>("TextAnalyzer")
    .smart_ptr<SharedPtr<TextAnalyzer>>("SharedPtr<TextAnalyzer>")
    .function("analyzeText", &TextAnalyzer::analyzeText)
    .function("analyzeLine",
      emscripten::optional_override([](SharedPtr<TextAnalyzer>& self, const U8String& text, const TextLineInfo& info) {
        LineAnalyzeResult result;
        self->analyzeLine(text, info, result);
        return result;
      })
    );

  emscripten::class_<DocumentAnalyzer>("DocumentAnalyzer")
    .smart_ptr<SharedPtr<DocumentAnalyzer>>("SharedPtr<DocumentAnalyzer>")
    .function("analyze", &DocumentAnalyzer::analyze)
    .function("analyzeIncremental", emscripten::select_overload<SharedPtr<DocumentHighlight>(const TextRange&, const U8String&) const>(&DocumentAnalyzer::analyzeIncremental))
    .function("analyzeIncremental", emscripten::select_overload<SharedPtr<DocumentHighlight>(size_t, size_t, const U8String&) const>(&DocumentAnalyzer::analyzeIncremental))
    .function("getDocument", &DocumentAnalyzer::getDocument);

  emscripten::class_<HighlightConfig>("HighlightConfig")
    .constructor<>()
    .property("showIndex", &HighlightConfig::show_index)
    .property("inlineStyle", &HighlightConfig::inline_style);

  emscripten::class_<SyntaxRule>("SyntaxRule")
    .smart_ptr<SharedPtr<SyntaxRule>>("SharedPtr<SyntaxRule>")
    .function("getName",
      emscripten::optional_override([](const SharedPtr<SyntaxRule>& self) {
        return self->name;
      })
    );

  emscripten::class_<HighlightEngine>("HighlightEngine")
    .smart_ptr<SharedPtr<HighlightEngine>>("SharedPtr<HighlightEngine>")
    .constructor<>(emscripten::optional_override([](const HighlightConfig& config) {
        return makeSharedPtr<HighlightEngine>(config);
      })
    )
    .function("registerStyleName", &HighlightEngine::registerStyleName)
    .function("getStyleName", &HighlightEngine::getStyleName)
    .function("compileSyntaxFromJson", &HighlightEngine::compileSyntaxFromJson)
    .function("compileSyntaxFromFile", &HighlightEngine::compileSyntaxFromFile)
    .function("getSyntaxRuleByName", &HighlightEngine::getSyntaxRuleByName)
    .function("getSyntaxRuleByExtension", &HighlightEngine::getSyntaxRuleByExtension)
    .function("createAnalyzerByName", &HighlightEngine::createAnalyzerByName)
    .function("createAnalyzerByExtension", &HighlightEngine::createAnalyzerByExtension)
    .function("loadDocument", &HighlightEngine::loadDocument)
    .function("removeDocument", &HighlightEngine::removeDocument);
}

}
