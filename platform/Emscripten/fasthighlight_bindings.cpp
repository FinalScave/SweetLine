#include <emscripten.h>
#include <emscripten/bind.h>

extern "C" {

EMSCRIPTEN_BINDINGS(lib) {
  emscripten::register_vector<String>("StringVector");
}

}