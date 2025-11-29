#include <jni.h>
#include "JEngine.hpp"

jint JNI_OnLoad(JavaVM *javaVm, void *) {
  JNIEnv *env = nullptr;
  jint result = javaVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
  if (result != JNI_OK) {
    return -1;
  }
  DocumentJni::RegisterMethods(env);
  SyntaxRuleJni::RegisterMethods(env);
  TextAnalyzerJni::RegisterMethods(env);
  DocumentAnalyzerJni::RegisterMethods(env);
  HighlightEngineJni::RegisterMethods(env);
  return JNI_VERSION_1_6;
}