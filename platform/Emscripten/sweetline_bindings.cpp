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
    .smart_ptr<Ptr<Document>>("Ptr<Document>")
    .constructor<>(emscripten::optional_override([](const String& uri, const String& content) {
        return MAKE_PTR<Document>(uri, content);
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
  emscripten::class_<TokenSpan>("TokenSpan")
    .constructor<>()
    .property("range", &TokenSpan::range)
    .property("style", &TokenSpan::style);
  BIND_LIST(TokenSpan, "TokenSpanList")
  emscripten::class_<LineHighlight>("LineHighlight")
    .constructor<>()
    .property("spans", &LineHighlight::spans)
    .function("toJson",emscripten::optional_override([](const Ptr<LineHighlight>& self) {
        String json;
        self->toJson(json);
        return json;
      })
    );
  emscripten::class_<DocumentHighlight>("DocumentHighlight")
    .smart_ptr<Ptr<DocumentHighlight>>("Ptr<DocumentHighlight>")
    .property("lines", &DocumentHighlight::lines)
    .function("toJson",emscripten::optional_override([](const Ptr<DocumentHighlight>& self) {
        String json;
        self->toJson(json);
        return json;
      })
    );
  emscripten::class_<DocumentAnalyzer>("DocumentAnalyzer")
    .smart_ptr<Ptr<DocumentAnalyzer>>("Ptr<DocumentAnalyzer>")
    .function("analyze", &DocumentAnalyzer::analyze)
    .function("analyzeChanges", emscripten::select_overload<Ptr<DocumentHighlight>(const TextRange&, const String&) const>(&DocumentAnalyzer::analyzeChanges))
    .function("analyzeChanges", emscripten::select_overload<Ptr<DocumentHighlight>(size_t, size_t, const String&) const>(&DocumentAnalyzer::analyzeChanges))
    .function("analyzeLine", &DocumentAnalyzer::analyzeLine)
    .function("getDocument", &DocumentAnalyzer::getDocument);
  emscripten::class_<HighlightConfig>("HighlightConfig")
    .constructor<>()
    .property("showIndex", &HighlightConfig::show_index);
  emscripten::class_<SyntaxRule>("SyntaxRule")
    .smart_ptr<Ptr<SyntaxRule>>("Ptr<SyntaxRule>")
    .function("getName",
      emscripten::optional_override([](const Ptr<SyntaxRule>& self) {
        return self->name;
      })
    );
  emscripten::class_<HighlightEngine>("HighlightEngine")
    .smart_ptr<Ptr<HighlightEngine>>("Ptr<HighlightEngine>")
    .constructor<>(emscripten::optional_override([](const HighlightConfig& config) {
        return MAKE_PTR<HighlightEngine>(config);
      })
    )
    .function("registerStyleName", &HighlightEngine::registerStyleName)
    .function("getStyleName", &HighlightEngine::getStyleName)
    .function("compileSyntaxFromJson", &HighlightEngine::compileSyntaxFromJson)
    .function("compileSyntaxFromFile", &HighlightEngine::compileSyntaxFromFile)
    .function("getSyntaxRuleByName", &HighlightEngine::getSyntaxRuleByName)
    .function("getSyntaxRuleByExtension", &HighlightEngine::getSyntaxRuleByExtension)
    .function("loadDocument", &HighlightEngine::loadDocument)
    .function("removeDocument", &HighlightEngine::removeDocument);
}

}
