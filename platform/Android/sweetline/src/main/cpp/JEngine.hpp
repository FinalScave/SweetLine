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
    const char* uri_str = env->GetStringUTFChars(uri, nullptr);
    const char* content_str = env->GetStringUTFChars(content, nullptr);
    jlong handle = makeCPtrHolderToHandle<jlong, Document>(uri_str, content_str);
    env->ReleaseStringUTFChars(uri, uri_str);
    env->ReleaseStringUTFChars(content, content_str);
    return handle;
  }

  static jstring getUri(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return env->NewStringUTF("");
    }
    return env->NewStringUTF(document->getUri().c_str());
  }

  static jint totalChars(jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->totalChars());
  }

  static jint getLineCharCount(jlong handle, jint line) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->getLineCharCount(line));
  }

  static jint charIndexOfLine(jlong handle, jint index) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->charIndexOfLine(index));
  }

  static jlong charIndexToPosition(jlong handle, jint index) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    TextPosition position = document->charIndexToPosition(index);
    return packTextPosition(position);
  }

  static jint getLineCount(jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return 0;
    }
    return static_cast<jint>(document->getLineCount());
  }

  static jstring getLine(JNIEnv* env, jclass clazz, jlong handle, jint line) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return {};
    }
    const U8String& line_text = document->getLine(line).text;
    return env->NewStringUTF(line_text.c_str());
  }

  static jstring getText(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(handle);
    if (document == nullptr) {
      return {};
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
    SharedPtr<SyntaxRule> rule = getCPtrHolderValue<jlong, SyntaxRule>(handle);
    if (rule == nullptr) {
      return env->NewStringUTF("");
    }
    return env->NewStringUTF(rule->name.c_str());
  }

  static jobjectArray getFileExtensions(JNIEnv* env, jclass clazz, jlong handle) {
    jclass string_class = env->FindClass("java/lang/String");
    SharedPtr<SyntaxRule> rule = getCPtrHolderValue<jlong, SyntaxRule>(handle);
    if (rule == nullptr) {
      return env->NewObjectArray(0, string_class, NULL);
    }
    const size_t ext_count = rule->file_extensions.size();
    jobjectArray array = env->NewObjectArray(ext_count, string_class, env->NewStringUTF(""));
    size_t index = 0;
    for (const U8String& extension: rule->file_extensions) {
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

static jintArray convertDocumentHighlightAsIntArray(JNIEnv* env, const SharedPtr<DocumentHighlight>& highlight,
                                                    const HighlightConfig& config) {
  size_t span_count = highlight->spanCount();
  size_t stride = computeSpanBufferStride(config);
  jintArray result = env->NewIntArray(static_cast<jsize>(2 + span_count * stride));
  if (result == nullptr) {
    return nullptr;
  }
  jint* buffer = env->GetIntArrayElements(result, nullptr);
  buffer[0] = static_cast<int32_t>(span_count);
  buffer[1] = static_cast<int32_t>(stride);
  writeDocumentHighlight(highlight, buffer + 2, config);
  env->ReleaseIntArrayElements(result, buffer, 0);
  return result;
}

// ====================================== TextAnalyzerJni ===========================================
class TextAnalyzerJni {
public:
  static void finalizeAnalyzer(jlong handle) {
    deleteCPtrHolder<jlong, TextAnalyzer>(handle);
  }

  static jintArray analyzeText(JNIEnv* env, jclass clazz, jlong handle, jstring text) {
    SharedPtr<TextAnalyzer> analyzer = getCPtrHolderValue<jlong, TextAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    const char* c_text = env->GetStringUTFChars(text, nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(c_text);
    env->ReleaseStringUTFChars(text, c_text);
    return convertDocumentHighlightAsIntArray(env, highlight, analyzer->getHighlightConfig());
  }

  static jintArray analyzeLine(JNIEnv* env, jclass clazz, jlong handle, jstring text, jintArray info) {
    SharedPtr<TextAnalyzer> analyzer = getCPtrHolderValue<jlong, TextAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    const char* c_text = env->GetStringUTFChars(text, nullptr);
    TextLineInfo line_info = unpackTextLineInfo(env->GetIntArrayElements(info, nullptr));
    LineAnalyzeResult analyze_result;
    analyzer->analyzeLine(c_text, line_info, analyze_result);
    env->ReleaseStringUTFChars(text, c_text);

    size_t span_count = analyze_result.highlight.spans.size();
    const HighlightConfig& config = analyzer->getHighlightConfig();
    size_t stride = computeSpanBufferStride(config);
    jintArray result = env->NewIntArray(static_cast<jsize>(4 + span_count * stride));
    if (result == nullptr) {
      return nullptr;
    }
    jint* buffer = env->GetIntArrayElements(result, nullptr);
    buffer[0] = static_cast<int32_t>(span_count);
    buffer[1] = static_cast<int32_t>(stride);
    buffer[2] = analyze_result.end_state;
    buffer[3] = static_cast<int32_t>(analyze_result.char_count);
    writeLineHighlight(analyze_result.highlight, buffer + 4, config);
    env->ReleaseIntArrayElements(result, buffer, 0);
    return result;
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/TextAnalyzer";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeFinalize", "(J)V", (void*) finalizeAnalyzer},
      {"nativeAnalyzeText", "(JLjava/lang/String;)[I", (void*) analyzeText},
      {"nativeAnalyzeLine", "(JLjava/lang/String;[I)[I", (void*) analyzeLine}
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
    deleteCPtrHolder<jlong, DocumentAnalyzer>(handle);
  }

  static jintArray analyze(JNIEnv* env, jclass clazz, jlong handle) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<jlong, DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
    return convertDocumentHighlightAsIntArray(env, highlight, analyzer->getHighlightConfig());
  }

  static jintArray analyzeChanges(JNIEnv* env, jclass clazz, jlong handle, jlong start_position, jlong end_position, jstring new_text) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<jlong, DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    TextRange range = {unpackTextPosition(start_position), unpackTextPosition(end_position)};
    const char* new_text_str = env->GetStringUTFChars(new_text, nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeIncremental(range, new_text_str);
    env->ReleaseStringUTFChars(new_text, new_text_str);
    return convertDocumentHighlightAsIntArray(env, highlight, analyzer->getHighlightConfig());
  }

  static jintArray analyzeChanges2(JNIEnv* env, jclass clazz, jlong handle, jint start_index, jint end_index, jstring new_text) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<jlong, DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return nullptr;
    }
    size_t unsigned_start_index = (size_t)start_index;
    size_t unsigned_end_index = (size_t)end_index;
    const char* new_text_str = env->GetStringUTFChars(new_text, nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeIncremental(unsigned_start_index, unsigned_end_index, new_text_str);
    env->ReleaseStringUTFChars(new_text, new_text_str);
    return convertDocumentHighlightAsIntArray(env, highlight, analyzer->getHighlightConfig());
  }

  static jlong getDocument(jlong handle) {
    SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<jlong, DocumentAnalyzer>(handle);
    if (analyzer == nullptr) {
      return 0;
    }
    return asCHandle<jlong>(analyzer->getDocument());
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/DocumentAnalyzer";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeFinalizeAnalyzer", "(J)V", (void*) finalizeAnalyzer},
      {"nativeAnalyze", "(J)[I", (void*) analyze},
      {"nativeAnalyzeChanges", "(JJJLjava/lang/String;)[I", (void*) analyzeChanges},
      {"nativeAnalyzeChanges2", "(JIILjava/lang/String;)[I", (void*) analyzeChanges2},
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
    deleteCPtrHolder<jlong, HighlightEngine>(handle);
  }

  static jlong makeEngine(jint config_bits) {
    HighlightConfig config = unpackHighlightConfig(config_bits);
    return makeCPtrHolderToHandle<jlong, HighlightEngine>(config);
  }

  static void registerStyleName(JNIEnv* env, jclass clazz, jlong handle, jstring name, jint id) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* name_str = env->GetStringUTFChars(name, nullptr);
    engine->registerStyleName(name_str, id);
    env->ReleaseStringUTFChars(name, name_str);
  }

  static jstring getStyleName(JNIEnv* env, jclass clazz, jlong handle, jint id) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return env->NewStringUTF("");
    }
    const U8String& name = engine->getStyleName(id);
    return env->NewStringUTF(name.c_str());
  }

  static void defineMacro(JNIEnv* env, jclass clazz, jlong handle, jstring macro_name) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* name_str = env->GetStringUTFChars(macro_name, JNI_FALSE);
    engine->defineMacro(name_str);
    env->ReleaseStringUTFChars(macro_name, name_str);
  }

  static void undefineMacro(JNIEnv* env, jclass clazz, jlong handle, jstring macro_name) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* name_str = env->GetStringUTFChars(macro_name, JNI_FALSE);
    engine->undefineMacro(name_str);
    env->ReleaseStringUTFChars(macro_name, name_str);
  }

  static jlong compileSyntaxFromJson(JNIEnv* env, jclass clazz, jlong handle, jstring json) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return 0;
    }
    const char* json_str = env->GetStringUTFChars(json, nullptr);
    SharedPtr<SyntaxRule> rule;
    try {
      rule = engine->compileSyntaxFromJson(json_str);
    } catch (SyntaxRuleParseError& error) {
      jclass ex_class = env->FindClass("com/qiplat/sweetline/SyntaxCompileError");
      env->ThrowNew(ex_class, StrUtil::formatString("%s: %s", error.what(), error.message().c_str()).c_str());
    }
    env->ReleaseStringUTFChars(json, json_str);
    return asCHandle<jlong>(rule);
  }

  static jlong compileSyntaxFromFile(JNIEnv* env, jclass clazz, jlong handle, jstring path) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
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
    env->ReleaseStringUTFChars(path, path_str);
    return asCHandle<jlong>(rule);
  }

  static jlong createAnalyzerByName(JNIEnv* env, jclass clazz, jlong handle, jstring syntax_name) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return 0;
    }
    const char* c_syntax_name = env->GetStringUTFChars(syntax_name, JNI_FALSE);
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName(c_syntax_name);
    env->ReleaseStringUTFChars(syntax_name, c_syntax_name);
    return asCHandle<jlong>(analyzer);
  }

  static jlong createAnalyzerByExtension(JNIEnv* env, jclass clazz, jlong handle, jstring extension) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return 0;
    }
    const char* c_extension = env->GetStringUTFChars(extension, JNI_FALSE);
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByExtension(c_extension);
    env->ReleaseStringUTFChars(extension, c_extension);
    return asCHandle<jlong>(analyzer);
  }

  static jlong loadDocument(jlong handle, jlong document_handle) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    SharedPtr<Document> document = getCPtrHolderValue<jlong, Document>(document_handle);
    if (engine == nullptr || document == nullptr) {
      return 0;
    }
    SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
    return asCHandle<jlong>(analyzer);
  }

  static void removeDocument(JNIEnv* env, jclass clazz, jlong handle, jstring uri) {
    SharedPtr<HighlightEngine> engine = getCPtrHolderValue<jlong, HighlightEngine>(handle);
    if (engine == nullptr) {
      return;
    }
    const char* uri_str = env->GetStringUTFChars(uri, JNI_FALSE);
    engine->removeDocument(uri_str);
    env->ReleaseStringUTFChars(uri, uri_str);
  }

  constexpr static const char *kJClassName = "com/qiplat/sweetline/HighlightEngine";
  constexpr static const JNINativeMethod kJMethods[] = {
      {"nativeMakeEngine", "(I)J", (void*) makeEngine},
      {"nativeFinalizeEngine", "(J)V", (void*) deleteEngine},
      {"nativeRegisterStyle", "(JLjava/lang/String;I)V", (void*) registerStyleName},
      {"nativeGetStyleName", "(JI)Ljava/lang/String;", (void*) getStyleName},
      {"nativeDefineMacro", "(JLjava/lang/String;)V", (void*) defineMacro},
      {"nativeUndefineMacro", "(JLjava/lang/String;)V", (void*) undefineMacro},
      {"nativeCompileSyntaxFromJson", "(JLjava/lang/String;)J", (void*) compileSyntaxFromJson},
      {"nativeCompileSyntaxFromFile", "(JLjava/lang/String;)J", (void*) compileSyntaxFromFile},
      {"nativeCreateAnalyzerByName", "(JLjava/lang/String;)J", (void*) createAnalyzerByName},
      {"nativeCreateAnalyzerByExtension", "(JLjava/lang/String;)J", (void*) createAnalyzerByExtension},
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
