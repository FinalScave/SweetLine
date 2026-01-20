@echo off
emcmake.bat cmake ../ -B ../cmake-build-emscripten -G "Ninja" -DCMAKE_BUILD_TYPE=Release && cmake --build ../cmake-build-emscripten