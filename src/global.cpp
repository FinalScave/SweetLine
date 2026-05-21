#include <oniguruma/oniguruma.h>
#include "macro.h"

namespace NS_SWEETLINE {
  class StaticInitializer {
  public:
    StaticInitializer() {
      OnigEncoding encodings[] = { ONIG_ENCODING_UTF8, ONIG_ENCODING_UTF16_LE };
      onig_initialize(encodings, std::size(encodings));
    }

    ~StaticInitializer() {
      onig_end();
    }
  };

  static StaticInitializer static_initializer();
}