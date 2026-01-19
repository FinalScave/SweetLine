#ifndef SWEETLINE_JENGINE_HPP
#define SWEETLINE_JENGINE_HPP

#include <jni.h>
#include "c_wrapper.hpp"
#include "util.h"

using namespace NS_SWEETLINE;

// ====================================== DocumentJni ===========================================
class DocumentJni {
public:
  static jlong makeDocument(JNIEnv* env, jclass clazz, jstring uri, jstring content) {
    const char* uri_str = env->GetStringUTFChars(uri, JNI_FALSE);
    const char* content_str = env->GetStringUTFChars(content, JNI_FALSE);
    return makeCPtrHolderToIntPtr<Document>(uri_str, content_str);
  }

  static jstring getUri(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return env->NewStringUTF("");
    }
    return env->NewStringUTF(document->getUri().c_str());
  }

  static jint totalChars(jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->totalChars());
  }

  static jint getLineCharCount(jlong handle, jint line) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->getLineCharCount(line));
  }

  static jint charIndexOfLine(jlong handle, jint index) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->charIndexOfLine(index));
  }

  static jlong charIndexToPosition(jlong handle, jint index) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    TextPosition position = document->charIndexToPosition(index);
    jlong line = (jlong)position.line;
    jlong column = (jlong)position.column;
    return (line << 32) | (column & 0XFFFFFFFFLL);
  }

  static jint getLineCount(jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->getLineCount());
  }

  static jstring getLine(JNIEnv* env, jclass clazz, jlong handle, jint line) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    const String& line_text = document->getLine(line).text;
    return env->NewStringUTF(line_text.c_str());
  }

  static jstring getText(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return env->NewStringUTF(document->getText().c_str());
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/Document";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeMakeDocument", "(Ljava/lang/String;Ljava/lang/String;)J", (void*) makeDocument},
      {"nativeGetUri", "(J)Ljava/lang/String;", (void*) getUri},
      {"nativeTotalChars", "(J)I", (void*) totalChars},
      {"nativeGetLineCharCount", "(JI)I", (void*) getLineCharCount},
      {"nativeCharIndexOfLine", "(JI)I", (void*) charIndexOfLine},
      {"nativeCharIndexToPosition", "(JI)J", (void*) charIndexToPosition},
      {"nativeGetLineCount", "(J)I", (void*) getLineCount},
      {"nativeGetLine", "(JI)Ljava/lang/String;", (void*) getLine},
      {"nativeGetText", "(J)Ljava/lang/String;", (void*) getText},
  };

  static void RegisterMethods(JNIEnv *env) {
    jclass java_class = env->FindClass(kJClassName);
    env->RegisterNatives(java_class, kJMethods,
                         sizeof(kJMethods) / sizeof(JNINativeMethod));
  }
};

// ====================================== SyntaxRuleJni ===========================================
class SyntaxRuleJni {
public:
  static jstring getName(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<SyntaxRule> rule = getCPtrHolderValue<SyntaxRule>(handle);
    if (rule == nullptr) {
      return env->NewStringUTF("");
    }
    return env->NewStringUTF(rule->name.c_str());
  }

  static jobjectArray getFileExtensions(JNIEnv* env, jclass clazz, jlong handle) {
    jclass string_class = env->FindClass("java/lang/String");
    SharedPtr<SyntaxRule> rule = getCPtrHolderValue<SyntaxRule>(handle);
    if (rule == nullptr) {
      return env->NewObjectArray(0, string_class, NULL);
    }
    const size_t ext_count = rule->file_extensions_.size();
    jobjectArray array = env->NewObjectArray(ext_count, string_class, env->NewStringUTF(""));
    size_t index = 0;
    for (const String& extension: rule->file_extensions_) {
      env->SetObjectArrayElement(array, index++, env->NewStringUTF(extension.c_str()));
    }
    return array;
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/SyntaxRule";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeGetName", "(J)Ljava/lang/String;", (void*) getName},
      {"nativeGetFileExtensions", "(J)[Ljava/lang/String;", (void*) getFileExtensions},
  };

  static void RegisterMethods(JNIEnv *env) {
    jclass java_class = env->FindClass(kJClassName);
    env->RegisterNatives(java_class, kJMethods,
                         sizeof(kJMethods) / sizeof(JNINativeMethod));
  }
};

// ====================================== DocumentAnalyzerJni ===========================================
class DocumentAnalyzerJni {
public:
  static void finalizeAnalyzer(jlong handle) {
    deleteCPtrHolder<DocumentAnalyzer>(handle);
  }

  static jintArray convertHighlightAsIntArray(JNIEnv* env, const SharedPtr<DocumentHighlight>& highlight) {
    size_t span_count = highlight->spanCount();
    jintArray result = env->NewIntArray(static_cast<jsize>(span_count * 7));
    if (result == nullptr) {
      return nullptr;
    }
    jint* buffer = env->GetIntArrayElements(result, JNI_FALSE);
    convertHighlightsToBuffer(highlight, buffer, true);
    env->ReleaseIntArrayElements(result, buffer, 0);
    return result;
  }

  static jintArray analyze(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
    return convertHighlightAsIntArray(env, highlight);
  }

  static jintArray analyzeChanges(JNIEnv* env, jclass clazz, jlong handle, jlong start_position, jlong end_position, jstring new_text) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    size_t start_line = (size_t)(jint)(start_position >> 32);
    size_t start_column = (size_t)(jint)(start_position & 0XFFFFFFFF);
    size_t end_line = (size_t)(jint)(end_position >> 32);
    size_t end_column = (size_t)(jint)(end_position & 0XFFFFFFFF);
    TextRange range = {{start_line, start_column}, {end_line, end_column}};
    const char* new_text_str = env->GetStringUTFChars(new_text, JNI_FALSE);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeChanges(range, new_text_str);
    return convertHighlightAsIntArray(env, highlight);
  }

  static jintArray analyzeChanges2(JNIEnv* env, jclass clazz, jlong handle, jint start_index, jint end_index, jstring new_text) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    size_t unsigned_start_index = (size_t)start_index;
    size_t unsigned_end_index = (size_t)end_index;
    const char* new_text_str = env->GetStringUTFChars(new_text, JNI_FALSE);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeChanges(unsigned_start_index, unsigned_end_index, new_text_str);
    return convertHighlightAsIntArray(env, highlight);
  }

  static jintArray analyzeLine(JNIEnv* env, jclass clazz, jlong handle, jint line) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    LineHighlight line_highlight;
    analyzer->analyzeLine((size_t)line, line_highlight);
    jintArray result = env->NewIntArray(static_cast<jsize>(line_highlight.spans.size() * 7));
    if (result == nullptr) {
      return nullptr;
    }
    jint* buffer = env->GetIntArrayElements(result, JNI_FALSE);
    size_t index = 0;
    for (const TokenSpan& span: line_highlight.spans) {
      buffer[index++] = static_cast<jint>(span.range.start.line);
      buffer[index++] = static_cast<jint>(span.range.start.column);
      buffer[index++] = static_cast<jint>(span.range.start.index);
      buffer[index++] = static_cast<jint>(span.range.end.line);
      buffer[index++] = static_cast<jint>(span.range.end.column);
      buffer[index++] = static_cast<jint>(span.range.end.index);
      buffer[index++] = static_cast<jint>(span.style);
    }
    env->ReleaseIntArrayElements(result, buffer, 0);
    return result;
  }

  static jlong getDocument(jlong handle) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return 0;
    }
    return toIntPtr(analyzer->getDocument());
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/DocumentAnalyzer";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeFinalizeAnalyzer", "(J)V", (void*) finalizeAnalyzer},
      {"nativeAnalyze", "(J)[I", (void*) analyze},
      {"nativeAnalyzeChanges", "(JJJLjava/lang/String;)[I", (void*) analyzeChanges},
      {"nativeAnalyzeChanges2", "(JIILjava/lang/String;)[I", (void*) analyzeChanges2},
      {"nativeAnalyzeLine", "(JI)[I", (void*) analyzeLine},
      {"nativeGetDocument", "(J)J", (void*) getDocument},
  };

  static void RegisterMethods(JNIEnv *env) {
    jclass java_class = env->FindClass(kJClassName);
    env->RegisterNatives(java_class, kJMethods,
                         sizeof(kJMethods) / sizeof(JNINativeMethod));
  }
};

// ====================================== HighlightEngineJni ===========================================
class HighlightEngineJni {
public:
  static void deleteEngine(jlong handle) {
    deleteCPtrHolder<HighlightEngine>(handle);
  }

  static jlong makeEngine(jlong handle, jboolean show_index) {
    HighlightConfig config = {static_cast<bool>(show_index)};
    return makeCPtrHolderToIntPtr<HighlightEngine>(config);
  }

  static void registerStyleName(JNIEnv* env, jclass clazz, jlong handle, jstring name, jint id) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* name_str = env->GetStringUTFChars(name, JNI_FALSE);
    engine->registerStyleName(name_str, id);
  }

  static jstring getStyleName(JNIEnv* env, jclass clazz, jlong handle, jint id) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    if (engine == nullptr) {
      return env->NewStringUTF("");
    }
    const String& name = engine->getStyleName(id);
    return env->NewStringUTF(name.c_str());
  }

  static jlong compileSyntaxFromJson(JNIEnv* env, jclass clazz, jlong handle, jstring json) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    if (engine == nullptr) {
      return 0;
    }
    const char* json_str = env->GetStringUTFChars(json, JNI_FALSE);
    SharedPtr<SyntaxRule> rule;
    try {
      rule = engine->compileSyntaxFromJson(json_str);
    } catch (SyntaxRuleParseError& error) {
      jclass ex_class = env->FindClass("com/qiplat/sweetline/SyntaxCompileError");
      env->ThrowNew(ex_class, StrUtil::formatString("%s: %s", error.what(), error.message().c_str()).c_str());
    }
    return toIntPtr(rule);
  }

  static jlong compileSyntaxFromFile(JNIEnv* env, jclass clazz, jlong handle, jstring path) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    if (engine == nullptr) {
      return 0;
    }
    const char* path_str = env->GetStringUTFChars(path, JNI_FALSE);
    SharedPtr<SyntaxRule> rule;
    try {
      rule = engine->compileSyntaxFromFile(path_str);
    } catch (SyntaxRuleParseError& error) {
      jclass ex_class = env->FindClass("com/qiplat/sweetline/SyntaxCompileError");
      env->ThrowNew(ex_class, StrUtil::formatString("%s: %s", error.what(), error.message().c_str()).c_str());
    }
    return toIntPtr(rule);
  }

  static jlong loadDocument(jlong handle, jlong document_handle) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    SharedPtr<Document> document = getCPtrHolderValue<Document>(document_handle);
    if (engine == nullptr || document == nullptr) {
      return 0;
    }
    SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
    return toIntPtr(analyzer);
  }

  static void removeDocument(JNIEnv* env, jclass clazz, jlong handle, jstring uri) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* uri_str = env->GetStringUTFChars(uri, JNI_FALSE);
    engine->removeDocument(uri_str);
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/HighlightEngine";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeMakeEngine", "(Z)J", (void*) makeEngine},
      {"nativeFinalizeEngine", "(J)V", (void*) deleteEngine},
      {"nativeRegisterStyle", "(JLjava/lang/String;I)V", (void*) registerStyleName},
      {"nativeGetStyleName", "(JI)Ljava/lang/String;", (void*) getStyleName},
      {"nativeCompileSyntaxFromJson", "(JLjava/lang/String;)J", (void*) compileSyntaxFromJson},
      {"nativeCompileSyntaxFromFile", "(JLjava/lang/String;)J", (void*) compileSyntaxFromFile},
      {"nativeLoadDocument", "(JJ)J", (void*) loadDocument},
      {"nativeRemoveDocument", "(JLjava/lang/String;)V", (void*) removeDocument},
  };

  static void RegisterMethods(JNIEnv *env) {
    jclass java_class = env->FindClass(kJClassName);
    env->RegisterNatives(java_class, kJMethods,
                         sizeof(kJMethods) / sizeof(JNINativeMethod));
  }
};

#endif //SWEETLINE_JENGINE_HPP
