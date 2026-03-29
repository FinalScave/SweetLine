#!/bin/bash
# Arguments:
# -b, --build           Build directory
# -o, --output          Output directory
# -s, --src             SweetLine project source directory
# -p, --platform        Target platform (all/android/windows/ohos/wasm; all means build everything)
# --android-ndk         Android NDK path
# --ohos-toolchain      OHOS toolchain CMake file path

# Parse arguments
PROJECT_DIR="../"
BUILD_DIR="../build"
OUTPUT_DIR="../prebuilt"
ANDROID_NDK="${ANDROID_NDK:-}"
OHOS_TOOLCHAIN="${OHOS_TOOLCHAIN:-}"
PLATFORM="all"
POSITIONAL_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    -b|--build)
      BUILD_DIR="$2"
      shift 2
      ;;
    -o|--output)
      OUTPUT_DIR="$2"
      shift 2
      ;;
    -s|--src)
      PROJECT_DIR="$2"
      shift 2
      ;;
    -p|--platform)
      PLATFORM="$2"
      shift 2
      ;;
    --android-ndk)
      ANDROID_NDK="$2"
      shift 2
      ;;
    --ohos-toolchain)
      OHOS_TOOLCHAIN="$2"
      shift 2
      ;;
    -*|--*)
      echo "Unknown option: $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1")
      shift
      ;;
  esac
done
set -- "${POSITIONAL_ARGS[@]}"

TARGET_NAME=sweetline
WASM_TARGET_NAME=sweetline
echo "============================= Start building: $PLATFORM ============================="

function resolve_android_strip_tool() {
  local host_tags=("windows-x86_64" "linux-x86_64" "darwin-x86_64" "darwin-arm64")
  local tag
  for tag in "${host_tags[@]}"; do
    local bin_dir="$ANDROID_NDK/toolchains/llvm/prebuilt/$tag/bin"
    if [ -x "$bin_dir/llvm-strip" ]; then
      echo "$bin_dir/llvm-strip"
      return 0
    fi
    if [ -x "$bin_dir/llvm-strip.exe" ]; then
      echo "$bin_dir/llvm-strip.exe"
      return 0
    fi
  done
  return 1
}

function strip_android_outputs() {
  local target_dir="$1"
  local strip_tool="$2"
  [ -d "$target_dir" ] || return 0
  [ -n "$strip_tool" ] || return 0
  while IFS= read -r -d '' so_file; do
    "$strip_tool" --strip-unneeded "$so_file"
  done < <(find "$target_dir" -type f -name "*.so" -print0)
}

function copy_built_libraries() {
  local build_dir="$1"
  local dest_dir="$2"
  mkdir -p "$dest_dir"
  find "$build_dir" -type f \( -name "*.dll" -o -name "*.so" -o -name "*.dylib" -o -name "*.wasm" -o -name "*.js" \) -exec cp -f {} "$dest_dir/" \;
}

function build_windows_msvc() {
  echo "============================= Windows X64 ============================="
  WINDOWS_BUILD_DIR="$BUILD_DIR/windows"
  WINDOWS_PREBUILT_DIR="$OUTPUT_DIR/windows/x64"
  cmake $PROJECT_DIR \
    -B $WINDOWS_BUILD_DIR \
    -G "Visual Studio 17 2022" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_FLAGS="/std:c++17 /EHsc /utf-8"
  cmake --build $WINDOWS_BUILD_DIR --target $TARGET_NAME -j 24 --config Release
  copy_built_libraries "$WINDOWS_BUILD_DIR/bin" "$WINDOWS_PREBUILT_DIR"
}

function build_osx() {
  OSX_ARCH=$1
  echo "============================= MacOSX $OSX_ARCH ============================="
  OSX_BUILD_DIR="$BUILD_DIR/osx/$OSX_ARCH"
  OSX_PREBUILT_DIR="$OUTPUT_DIR/osx/$OSX_ARCH"
  cmake $PROJECT_DIR \
    -B $OSX_BUILD_DIR \
    -G "Ninja" \
    -DCMAKE_CXX_FLAGS="-std=c++17" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC_LIB=OFF \
    -DBUILD_TESTING=OFF \
    -DCMAKE_OSX_ARCHITECTURES="$OSX_ARCH"
  cmake --build $OSX_BUILD_DIR --target $TARGET_NAME -j 12
  copy_built_libraries "$OSX_BUILD_DIR/lib" "$OSX_PREBUILT_DIR"
}

function build_linux() {
    LINUX_ARCH=$1
    LINUX_BUILD_DIR="$BUILD_DIR/linux/$LINUX_ARCH"
    if [ $LINUX_ARCH = "aarch64" ]; then
      LINUX_PREBUILT_DIR="$OUTPUT_DIR/linux/aarch64"
    elif [ $LINUX_ARCH = "x86_64" ]; then
      LINUX_PREBUILT_DIR="$OUTPUT_DIR/linux/x86_64"
    else
      echo "Unsupported arch: $LINUX_ARCH"
      return 1
    fi
    cmake $PROJECT_DIR \
      -B $LINUX_BUILD_DIR \
      -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++17 -fPIC" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_STATIC_LIB=OFF \
      -DBUILD_TESTING=OFF
    cmake --build $LINUX_BUILD_DIR --target $TARGET_NAME -j 12
    copy_built_libraries "$LINUX_BUILD_DIR/lib" "$LINUX_PREBUILT_DIR"
}

function build_emscripten() {
  echo "============================= WebAssembly ============================="
  WASM_BUILD_DIR="$BUILD_DIR/emscripten"
  WASM_PREBUILT_DIR="$OUTPUT_DIR/wasm"
  emcmake.bat cmake $PROJECT_DIR\
    -B $WASM_BUILD_DIR \
    -G "Ninja" \
    -DCMAKE_CXX_FLAGS="-std=c++17" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC_LIB=OFF \
    -DBUILD_TESTING=OFF
  cmake --build $WASM_BUILD_DIR --target $WASM_TARGET_NAME -j 24
  copy_built_libraries "$WASM_BUILD_DIR/bin" "$WASM_PREBUILT_DIR"
}

function build_android() {
  ANDROID_ARCH=$1
  if [ -z "$ANDROID_NDK" ]; then
    echo "ANDROID_NDK is not set. Use --android-ndk or export ANDROID_NDK."
    exit 1
  fi
  echo "============================= Android $ANDROID_ARCH ============================="
  echo "============================= NDK: $ANDROID_NDK ============================="
  ANDROID_BUILD_DIR="$BUILD_DIR/android/$ANDROID_ARCH"
  ANDROID_PREBUILT_DIR="$OUTPUT_DIR/android/$ANDROID_ARCH"
  cmake $PROJECT_DIR \
    -B $ANDROID_BUILD_DIR \
    -G "Ninja" \
    -DANDROID_ABI=$ANDROID_ARCH \
    -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ARCH \
    -DANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_PLATFORM=android-21 \
    -DCMAKE_CXX_FLAGS="-std=c++17" \
    -DBUILD_STATIC_LIB=OFF \
    -DBUILD_TESTING=OFF
  cmake --build $ANDROID_BUILD_DIR --target $TARGET_NAME -j 24

  local strip_tool
  strip_tool=$(resolve_android_strip_tool)
  if [ -n "$strip_tool" ]; then
    echo "============================= Stripping Android .so ($ANDROID_ARCH) ============================="
    echo "============================= Strip Tool: $strip_tool ============================="
    strip_android_outputs "$ANDROID_BUILD_DIR" "$strip_tool"
    copy_built_libraries "$ANDROID_BUILD_DIR" "$ANDROID_PREBUILT_DIR"
  else
    echo "Error: llvm-strip not found under ANDROID_NDK=$ANDROID_NDK, cannot stripping."
    return 1
  fi
}

function build_ohos() {
  OHOS_ARCH=$1
  if [ -z "$OHOS_TOOLCHAIN" ]; then
    echo "OHOS_TOOLCHAIN is not set. Use --ohos-toolchain or export OHOS_TOOLCHAIN."
    exit 1
  fi
  echo "============================= OHOS $OHOS_ARCH ============================="
  echo "============================= Toolchain: $OHOS_TOOLCHAIN ============================="
  OHOS_BUILD_DIR="$BUILD_DIR/ohos/$OHOS_ARCH"
  OHOS_PREBUILT_DIR="$OUTPUT_DIR/ohos/$OHOS_ARCH"
  cmake $PROJECT_DIR \
    -B $OHOS_BUILD_DIR \
    -G "Ninja" \
    -DOHOS_PLATFORM=OHOS \
    -DOHOS_ARCH=$OHOS_ARCH \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$OHOS_TOOLCHAIN" \
    -DCMAKE_CXX_FLAGS="-std=c++17" \
    -DBUILD_STATIC_LIB=OFF \
    -DBUILD_TESTING=OFF
  cmake --build $OHOS_BUILD_DIR --target $TARGET_NAME -j 24
  copy_built_libraries "$OHOS_BUILD_DIR/lib" "$OHOS_PREBUILT_DIR"
}

if [ $PLATFORM = "all" ]; then
  build_windows_msvc
  build_linux x86_64
  build_android arm64-v8a
  build_android x86_64
  build_ohos arm64-v8a
  build_ohos x86_64
elif [ $PLATFORM = "wasm" ]; then
  build_emscripten
elif [ $PLATFORM = "windows" ]; then
  build_windows_msvc
elif [ $PLATFORM = "osx" ]; then
  build_osx arm64
  build_osx x86_64
elif [ $PLATFORM = "linux" ]; then
  build_linux x86_64
elif [ $PLATFORM = "android" ]; then
  build_android arm64-v8a
  build_android x86_64
elif [ $PLATFORM = "ohos" ]; then
  build_ohos arm64-v8a
  build_ohos x86_64
fi
