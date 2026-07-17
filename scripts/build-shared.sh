#!/bin/bash
# Arguments:
# -b, --build           Build directory
# -o, --output          Output directory
# -s, --src             SweetLine project source directory
# -p, --platform        Target platform (all/android/linux/osx/ios/ohos/wasm)
# --android-ndk         Android NDK path
# --ohos-toolchain      OHOS toolchain CMake file path

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/.."
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_DIR="$PROJECT_DIR/prebuilt"
ANDROID_NDK="${ANDROID_NDK:-}"
OHOS_TOOLCHAIN="${OHOS_TOOLCHAIN:-}"
PLATFORM="all"

parse_build_shared_args() {
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
}

# Shared Apple helpers
APPLE_CORE_MODULE_NAME="SweetLineCore"
APPLE_FRAMEWORK_BUNDLE_NAME="${APPLE_CORE_MODULE_NAME}.framework"
APPLE_XCFRAMEWORK_BUNDLE_NAME="${APPLE_CORE_MODULE_NAME}.xcframework"
APPLE_XCFRAMEWORK_IOS_ARCHIVE_NAME="${APPLE_CORE_MODULE_NAME}-iOS.xcframework.zip"
APPLE_XCFRAMEWORK_OSX_ARCHIVE_NAME="${APPLE_CORE_MODULE_NAME}-macOS.xcframework.zip"

apple_runtime_prebuilt_dir() {
  local output_root="$1"
  local platform="$2"
  local arch="$3"

  case "$platform" in
    ios|osx)
      printf '%s/%s/%s\n' "$output_root" "$platform" "$arch"
      ;;
    *)
      echo "Unsupported Apple platform: $platform" >&2
      return 1
      ;;
  esac
}

find_apple_framework_path() {
  local build_dir="$1"
  local framework_name="${2:-$APPLE_CORE_MODULE_NAME}"
  local framework_path=""
  local candidates=(
    "$build_dir/lib/$framework_name.framework"
    "$build_dir/lib/Release/$framework_name.framework"
    "$build_dir/lib/Release-iphoneos/$framework_name.framework"
    "$build_dir/lib/Release-iphonesimulator/$framework_name.framework"
    "$build_dir/Release/$framework_name.framework"
    "$build_dir/Release-iphoneos/$framework_name.framework"
    "$build_dir/Release-iphonesimulator/$framework_name.framework"
    "$build_dir/$framework_name.framework"
  )

  for framework_path in "${candidates[@]}"; do
    if [ -d "$framework_path" ]; then
      printf '%s\n' "$framework_path"
      return 0
    fi
  done

  return 1
}

find_apple_dynamic_lib_path() {
  local build_dir="$1"
  local dynamic_lib_name="${2:-libsweetline.dylib}"
  local dynamic_lib_path=""
  local pattern=""
  local candidates=(
    "$build_dir/$dynamic_lib_name"
    "$build_dir/Release/$dynamic_lib_name"
    "$build_dir/Release-iphoneos/$dynamic_lib_name"
    "$build_dir/Release-iphonesimulator/$dynamic_lib_name"
    "$build_dir/lib/$dynamic_lib_name"
    "$build_dir/lib/Release/$dynamic_lib_name"
    "$build_dir/lib/Release-iphoneos/$dynamic_lib_name"
    "$build_dir/lib/Release-iphonesimulator/$dynamic_lib_name"
  )

  for dynamic_lib_path in "${candidates[@]}"; do
    if [ -f "$dynamic_lib_path" ]; then
      printf '%s\n' "$dynamic_lib_path"
      return 0
    fi
  done

  shopt -s nullglob
  for pattern in \
    "$build_dir/build"'/*/Release/'"$dynamic_lib_name" \
    "$build_dir/build"'/*/Release-iphoneos/'"$dynamic_lib_name" \
    "$build_dir/build"'/*/Release-iphonesimulator/'"$dynamic_lib_name"; do
    for dynamic_lib_path in $pattern; do
      if [ -f "$dynamic_lib_path" ]; then
        shopt -u nullglob
        printf '%s\n' "$dynamic_lib_path"
        return 0
      fi
    done
  done
  shopt -u nullglob

  return 1
}

apple_prerequisite_status() {
  if [ "$(uname -s)" != "Darwin" ]; then
    printf '%s\n' "Apple build skipped: only supported on macOS"
    return 1
  fi

  if ! command -v xcodebuild >/dev/null 2>&1; then
    printf '%s\n' "Apple build skipped: xcodebuild not found"
    return 1
  fi

  if ! command -v ditto >/dev/null 2>&1; then
    printf '%s\n' "Apple build skipped: ditto not found"
    return 1
  fi

  if ! command -v lipo >/dev/null 2>&1; then
    printf '%s\n' "Apple build skipped: lipo not found"
    return 1
  fi

  return 0
}

verify_file_exists() {
  local file_path="$1"
  local label="$2"

  if [ ! -e "$file_path" ]; then
    echo "$label not found at $file_path" >&2
    return 1
  fi

  return 0
}

resolve_framework_binary_path() {
  local framework_dir="$1"
  local binary_name="${2:-$APPLE_CORE_MODULE_NAME}"

  if [ -f "$framework_dir/Versions/A/$binary_name" ]; then
    printf '%s\n' "$framework_dir/Versions/A/$binary_name"
    return 0
  fi

  if [ -f "$framework_dir/$binary_name" ]; then
    printf '%s\n' "$framework_dir/$binary_name"
    return 0
  fi

  return 1
}

archive_framework_bundle() {
  local framework_dir="$1"
  local output_dir="$2"
  local framework_name="${3:-$APPLE_CORE_MODULE_NAME}"
  local framework_zip="$output_dir/$framework_name.framework.zip"

  if [ ! -d "$framework_dir" ]; then
    echo "Framework bundle not found at $framework_dir" >&2
    return 1
  fi

  mkdir -p "$output_dir"
  rm -f "$framework_zip"

  ditto -c -k --norsrc --keepParent "$framework_dir" "$framework_zip"

  verify_file_exists "$framework_zip" "$framework_name framework archive"
}

copy_apple_runtime_library() {
  local source_path="$1"
  local dest_dir="$2"
  local label="$3"
  local dest_path=""

  verify_file_exists "$source_path" "$label" || return 1
  mkdir -p "$dest_dir"
  dest_path="$dest_dir/$(basename "$source_path")"
  cp -f "$source_path" "$dest_path"
  verify_file_exists "$dest_path" "$label copy"
}

copy_built_apple_dynamic_lib() {
  local build_dir="$1"
  local dest_dir="$2"
  local dynamic_lib_name="${3:-libsweetline.dylib}"
  local source_path=""

  source_path="$(find_apple_dynamic_lib_path "$build_dir" "$dynamic_lib_name")" || {
    echo "Apple dynamic library not found under $build_dir" >&2
    return 1
  }

  copy_apple_runtime_library "$source_path" "$dest_dir" "Apple dynamic library"
}

archive_built_apple_framework() {
  local build_dir="$1"
  local dest_dir="$2"
  local framework_name="${3:-$APPLE_CORE_MODULE_NAME}"
  local source_path=""

  source_path="$(find_apple_framework_path "$build_dir" "$framework_name")" || {
    echo "Apple framework not found under $build_dir" >&2
    return 1
  }

  archive_framework_bundle "$source_path" "$dest_dir" "$framework_name"
}

copy_xcframework() {
  local apple_binaries_dir="$1"
  local output_dir="$2"
  local source_xcframework_name="$3"
  local archive_name="${4:-${source_xcframework_name}.zip}"
  local xcframework_dir="${apple_binaries_dir}/${source_xcframework_name}"
  local xcframework_zip="$output_dir/${archive_name}"

  if [ ! -d "$xcframework_dir" ]; then
    echo "XCFramework not found at $xcframework_dir" >&2
    return 1
  fi

  mkdir -p "$output_dir"
  rm -f "$xcframework_zip"
  ditto -c -k --norsrc --keepParent "$xcframework_dir" "$xcframework_zip"
}

create_apple_xcframework() {
  local output_path="$1"
  shift
  local cmd=(xcodebuild -create-xcframework)

  rm -rf "$output_path"
  cmd+=("$@")
  cmd+=(-output "$output_path")
  "${cmd[@]}"
}

create_osx_universal_framework() {
  local arm64_build_dir="$1"
  local x64_build_dir="$2"
  local output_dir="$3"
  local framework_name="${4:-$APPLE_CORE_MODULE_NAME}"
  local arm64_framework=""
  local x64_framework=""
  local arm64_binary=""
  local x64_binary=""
  local universal_framework="$output_dir/$framework_name.framework"
  local universal_binary=""

  arm64_framework="$(find_apple_framework_path "$arm64_build_dir" "$framework_name")" || {
    echo "macOS arm64 framework not found under $arm64_build_dir" >&2
    return 1
  }
  x64_framework="$(find_apple_framework_path "$x64_build_dir" "$framework_name")" || {
    echo "macOS x86_64 framework not found under $x64_build_dir" >&2
    return 1
  }

  arm64_binary="$(resolve_framework_binary_path "$arm64_framework" "$framework_name")" || {
    echo "macOS arm64 framework binary not found in $arm64_framework" >&2
    return 1
  }
  x64_binary="$(resolve_framework_binary_path "$x64_framework" "$framework_name")" || {
    echo "macOS x86_64 framework binary not found in $x64_framework" >&2
    return 1
  }

  mkdir -p "$output_dir"
  rm -rf "$universal_framework"
  ditto "$arm64_framework" "$universal_framework"

  universal_binary="$(resolve_framework_binary_path "$universal_framework" "$framework_name")" || {
    echo "macOS universal framework binary location not found in $universal_framework" >&2
    return 1
  }

  rm -f "$universal_binary"
  lipo -create "$arm64_binary" "$x64_binary" -output "$universal_binary"
  verify_file_exists "$universal_framework" "macOS universal framework"
  verify_file_exists "$universal_binary" "macOS universal framework binary"
}

TARGET_NAME=sweetline
WASM_TARGET_NAME=sweetline

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

function build_apple() {
  local apple_build_dir="$1"
  local apple_prebuilt_dir="$2"
  local apple_sysroot="$3"
  local apple_arch="$4"
  local apple_target_name="$5"
  local apple_generator="$6"
  local apple_system_name="$7"
  shift 7

  local cmake_args=(
    "$PROJECT_DIR"
    -B "$apple_build_dir"
    -G "$apple_generator"
    -DCMAKE_CXX_FLAGS=-std=c++17
    -DCMAKE_BUILD_TYPE=Release
    -DSWEETLINE_BUILD_TESTS=OFF
    -DSWEETLINE_BUILD_APPLE_FRAMEWORK=ON
    -DSWEETLINE_BUILD_SHARED=ON
    -DSWEETLINE_BUILD_STATIC=OFF
    -DSWEETLINE_APPLE_STATIC_FRAMEWORK=OFF
    -DCMAKE_OSX_SYSROOT="$apple_sysroot"
    -DCMAKE_OSX_ARCHITECTURES="$apple_arch"
  )

  if [ -n "$apple_system_name" ]; then
    cmake_args+=( -DCMAKE_SYSTEM_NAME="$apple_system_name" )
  fi

  if [ $# -gt 0 ]; then
    cmake_args+=( "$@" )
  fi

  cmake "${cmake_args[@]}"
  cmake --build "$apple_build_dir" --target "$apple_target_name" --config Release -j 12
  archive_built_apple_framework "$apple_build_dir" "$apple_prebuilt_dir" "$APPLE_CORE_MODULE_NAME"
}

function build_apple_runtime_libs() {
  local apple_build_dir="$1"
  local apple_prebuilt_dir="$2"
  local apple_sysroot="$3"
  local apple_arch="$4"
  local apple_generator="$5"
  local apple_system_name="$6"
  shift 6

  local cmake_args=(
    "$PROJECT_DIR"
    -B "$apple_build_dir"
    -G "$apple_generator"
    -DCMAKE_CXX_FLAGS=-std=c++17
    -DCMAKE_BUILD_TYPE=Release
    -DSWEETLINE_BUILD_SHARED=ON
    -DSWEETLINE_BUILD_STATIC=OFF
    -DSWEETLINE_BUILD_TESTS=OFF
    -DSWEETLINE_BUILD_APPLE_FRAMEWORK=OFF
    -DCMAKE_OSX_SYSROOT="$apple_sysroot"
    -DCMAKE_OSX_ARCHITECTURES="$apple_arch"
  )

  if [ -n "$apple_system_name" ]; then
    cmake_args+=( -DCMAKE_SYSTEM_NAME="$apple_system_name" )
  fi

  if [ $# -gt 0 ]; then
    cmake_args+=( "$@" )
  fi

  cmake "${cmake_args[@]}"
  cmake --build "$apple_build_dir" --target "$TARGET_NAME" --config Release -j 12
  copy_built_apple_dynamic_lib "$apple_build_dir" "$apple_prebuilt_dir"
}

function build_osx() {
  local osx_arch="$1"
  local osx_build_dir="$BUILD_DIR/osx/$osx_arch"
  local osx_runtime_build_dir="$BUILD_DIR/osx-runtime/$osx_arch"
  local osx_prebuilt_dir

  osx_prebuilt_dir="$(apple_runtime_prebuilt_dir "$OUTPUT_DIR" osx "$osx_arch")"
  echo "============================= MacOSX $osx_arch ============================="
  build_apple_runtime_libs "$osx_runtime_build_dir" "$osx_prebuilt_dir" "macosx" "$osx_arch" "Xcode" "" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
  build_apple "$osx_build_dir" "$osx_prebuilt_dir" "macosx" "$osx_arch" "$APPLE_CORE_MODULE_NAME" "Xcode" "" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
}

function build_ios() {
  local ios_target="$1"
  local ios_arch=""
  local ios_sdk=""
  local ios_build_dir="$BUILD_DIR/ios-xcode/$ios_target"
  local ios_prebuilt_dir=""
  local ios_framework_build_dir="$BUILD_DIR/apple-framework/ios/$ios_target"

  if [[ "$ios_target" == simulator-* ]]; then
    ios_arch="${ios_target#simulator-}"
    ios_sdk="iphonesimulator"
  else
    ios_arch="$ios_target"
    ios_sdk="iphoneos"
  fi

  ios_prebuilt_dir="$(apple_runtime_prebuilt_dir "$OUTPUT_DIR" ios "$ios_target")"
  echo "============================= iOS $ios_target ============================="
  rm -f "$ios_prebuilt_dir/libsweetline_static.a"

  build_apple_runtime_libs "$ios_build_dir" "$ios_prebuilt_dir" "$ios_sdk" "$ios_arch" "Xcode" "iOS" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY= \
    -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=
  build_apple "$ios_framework_build_dir" "$ios_prebuilt_dir" "$ios_sdk" "$ios_arch" "$APPLE_CORE_MODULE_NAME" "Xcode" "iOS" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY= \
    -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=
}

function build_apple_xcframework() {
  local apple_scope="$1"
  local output_dir="$2"
  local archive_name="$3"
  local label="$4"
  local apple_binaries_dir="$BUILD_DIR/apple-xcframework/binaries"
  local xcframework_dir="$apple_binaries_dir/$APPLE_XCFRAMEWORK_BUNDLE_NAME"
  local xcframework_zip="$output_dir/$archive_name"

  echo "============================= $label XCFramework ============================="
  mkdir -p "$output_dir"
  mkdir -p "$apple_binaries_dir"
  rm -rf "$xcframework_dir"

  if [ "$apple_scope" = "osx" ]; then
    local universal_dir="$BUILD_DIR/apple-xcframework/osx-universal"
    local universal_framework="$universal_dir/$APPLE_FRAMEWORK_BUNDLE_NAME"

    create_osx_universal_framework "$BUILD_DIR/osx/arm64" "$BUILD_DIR/osx/x86_64" "$universal_dir" "$APPLE_CORE_MODULE_NAME"
    create_apple_xcframework "$xcframework_dir" \
      -framework "$universal_framework"
  elif [ "$apple_scope" = "ios" ]; then
    local ios_device_framework=""
    local ios_simulator_framework=""

    ios_device_framework="$(find_apple_framework_path "$BUILD_DIR/apple-framework/ios/arm64" "$APPLE_CORE_MODULE_NAME")" || {
      echo "iOS device framework not found under $BUILD_DIR/apple-framework/ios/arm64" >&2
      return 1
    }
    ios_simulator_framework="$(find_apple_framework_path "$BUILD_DIR/apple-framework/ios/simulator-arm64" "$APPLE_CORE_MODULE_NAME")" || {
      echo "iOS simulator framework not found under $BUILD_DIR/apple-framework/ios/simulator-arm64" >&2
      return 1
    }

    verify_file_exists "$ios_device_framework" "iOS device framework" || return 1
    verify_file_exists "$ios_simulator_framework" "iOS simulator framework" || return 1

    create_apple_xcframework "$xcframework_dir" \
      -framework "$ios_device_framework" \
      -framework "$ios_simulator_framework"
  else
    echo "Unsupported Apple XCFramework scope: $apple_scope" >&2
    return 1
  fi

  copy_xcframework "$apple_binaries_dir" "$output_dir" "$APPLE_XCFRAMEWORK_BUNDLE_NAME" "$archive_name"
  verify_file_exists "$xcframework_dir" "$label XCFramework directory"
  verify_file_exists "$xcframework_zip" "$label XCFramework archive"
}

function build_linux() {
    local linux_arch="$1"
    local linux_build_dir="$BUILD_DIR/linux/$linux_arch"
    local linux_prebuilt_dir=""
    local linux_toolchain_args=()
    local linux_cache_file="$linux_build_dir/CMakeCache.txt"

    if [ "$linux_arch" = "aarch64" ]; then
      linux_prebuilt_dir="$OUTPUT_DIR/linux/aarch64"
      command -v aarch64-linux-gnu-gcc >/dev/null 2>&1 || {
        echo "Missing aarch64-linux-gnu-gcc. Install gcc-aarch64-linux-gnu." >&2
        return 1
      }
      command -v aarch64-linux-gnu-g++ >/dev/null 2>&1 || {
        echo "Missing aarch64-linux-gnu-g++. Install g++-aarch64-linux-gnu." >&2
        return 1
      }
      if [ ! -f "$linux_cache_file" ]; then
        linux_toolchain_args+=("-DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/cmake/linux-aarch64-toolchain.cmake")
      fi
    elif [ "$linux_arch" = "x86_64" ]; then
      linux_prebuilt_dir="$OUTPUT_DIR/linux/x86_64"
    else
      echo "Unsupported arch: $linux_arch"
      return 1
    fi

    cmake "$PROJECT_DIR" \
      -B "$linux_build_dir" \
      -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++17 -fPIC" \
      -DCMAKE_BUILD_TYPE=Release \
      -DSWEETLINE_BUILD_STATIC=OFF \
      -DSWEETLINE_BUILD_TESTS=OFF \
      "${linux_toolchain_args[@]}"
    cmake --build "$linux_build_dir" --target "$TARGET_NAME" -j 12
    copy_built_libraries "$linux_build_dir/lib" "$linux_prebuilt_dir"
}

function build_emscripten() {
  echo "============================= WebAssembly ============================="
  WASM_BUILD_DIR="$BUILD_DIR/emscripten"
  WASM_PREBUILT_DIR="$OUTPUT_DIR/wasm"
  emcmake cmake "$PROJECT_DIR" \
    -B "$WASM_BUILD_DIR" \
    -G "Ninja" \
    -DCMAKE_CXX_FLAGS="-std=c++17" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSWEETLINE_BUILD_STATIC=OFF \
    -DSWEETLINE_BUILD_TESTS=OFF
  cmake --build "$WASM_BUILD_DIR" --target "$WASM_TARGET_NAME" -j 24
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
    -DSWEETLINE_BUILD_STATIC=OFF \
    -DSWEETLINE_BUILD_TESTS=OFF \
    -DSWEETLINE_BUILD_ANDROID_JNI=OFF
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
    -DSWEETLINE_BUILD_STATIC=OFF \
    -DSWEETLINE_BUILD_TESTS=OFF
  cmake --build $OHOS_BUILD_DIR --target $TARGET_NAME -j 24
  copy_built_libraries "$OHOS_BUILD_DIR/lib" "$OHOS_PREBUILT_DIR"
}

run_build_shared() {
  parse_build_shared_args "$@"
  echo "============================= Start building: $PLATFORM ============================="

  if [ "$PLATFORM" = "all" ]; then
    if [ "$(uname -s)" = "Darwin" ]; then
      apple_prerequisite_status >/dev/null || {
        apple_prerequisite_status >&2
        exit 1
      }
      build_osx arm64
      build_osx x86_64
      build_apple_xcframework osx "$OUTPUT_DIR/osx" "$APPLE_XCFRAMEWORK_OSX_ARCHIVE_NAME" "macOS"
      build_ios arm64
      build_ios simulator-arm64
      build_apple_xcframework ios "$OUTPUT_DIR/ios" "$APPLE_XCFRAMEWORK_IOS_ARCHIVE_NAME" "iOS"
    elif [ "$(uname -s)" = "Linux" ]; then
      build_linux x86_64
      build_linux aarch64
    else
      echo "build-shared.sh supports all only on macOS or Linux. Use build-shared.ps1 on Windows." >&2
      exit 1
    fi
  elif [ "$PLATFORM" = "wasm" ]; then
    build_emscripten
  elif [ "$PLATFORM" = "osx" ]; then
    apple_prerequisite_status >/dev/null || {
      apple_prerequisite_status >&2
      exit 1
    }
    build_osx arm64
    build_osx x86_64
    build_apple_xcframework osx "$OUTPUT_DIR/osx" "$APPLE_XCFRAMEWORK_OSX_ARCHIVE_NAME" "macOS"
  elif [ "$PLATFORM" = "ios" ]; then
    apple_prerequisite_status >/dev/null || {
      apple_prerequisite_status >&2
      exit 1
    }
    build_ios arm64
    build_ios simulator-arm64
    build_apple_xcframework ios "$OUTPUT_DIR/ios" "$APPLE_XCFRAMEWORK_IOS_ARCHIVE_NAME" "iOS"
  elif [ "$PLATFORM" = "linux" ]; then
    if [ "$(uname -s)" != "Linux" ]; then
      echo "Linux builds require a Linux host. Use build-shared.ps1 -UseWsl on Windows." >&2
      exit 1
    fi
    build_linux x86_64
    build_linux aarch64
  elif [ "$PLATFORM" = "android" ]; then
    build_android arm64-v8a
    build_android x86_64
  elif [ "$PLATFORM" = "ohos" ]; then
    build_ohos arm64-v8a
    build_ohos x86_64
  else
    echo "Unsupported platform: $PLATFORM" >&2
    exit 1
  fi
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  run_build_shared "$@"
fi
