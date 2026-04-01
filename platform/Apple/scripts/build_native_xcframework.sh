#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPLE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="${SWEETLINE_REPO_ROOT:-$(cd "${APPLE_DIR}/../.." && pwd)}"
APPLE_BUILD_ROOT="${SWEETLINE_APPLE_BUILD_ROOT:-${REPO_ROOT}/build}"
OUTPUT_DIR="${SWEETLINE_APPLE_OUTPUT_DIR:-${APPLE_DIR}/binaries}"
BUILD_SCOPE="${1:-all}"

source "${REPO_ROOT}/scripts/build-shared.sh"

REPO_ROOT="${SWEETLINE_REPO_ROOT:-$(cd "${APPLE_DIR}/../.." && pwd)}"
APPLE_BUILD_ROOT="${SWEETLINE_APPLE_BUILD_ROOT:-${REPO_ROOT}/build}"
OUTPUT_DIR="${SWEETLINE_APPLE_OUTPUT_DIR:-${APPLE_DIR}/binaries}"
BUILD_SCOPE="${1:-all}"

if [[ "${BUILD_SCOPE}" = "all" && $# -ge 2 ]]; then
  echo "Custom output name is not supported for build scope 'all'" >&2
  exit 1
fi

case "${BUILD_SCOPE}" in
  ios)
    OUTPUT_NAME="${2:-${APPLE_XCFRAMEWORK_IOS_NAME}}"
    ;;
  osx)
    OUTPUT_NAME="${2:-${APPLE_XCFRAMEWORK_OSX_NAME}}"
    ;;
  all)
    OUTPUT_NAME=""
    ;;
esac

OUTPUT_XCFRAMEWORK="${OUTPUT_NAME:+${OUTPUT_DIR}/${OUTPUT_NAME}}"
FRAMEWORK_NAME="${APPLE_FRAMEWORK_NAME}"
TARGET_NAME="sweetline"

MACOS_ARM64_BUILD_DIR="${APPLE_BUILD_ROOT}/apple-macos-arm64"
MACOS_X64_BUILD_DIR="${APPLE_BUILD_ROOT}/apple-macos-x86_64"
MACOS_UNIVERSAL_DIR="${APPLE_BUILD_ROOT}/apple-macos-universal"
IOS_DEVICE_BUILD_DIR="${APPLE_BUILD_ROOT}/apple-ios-device"
IOS_SIM_BUILD_DIR="${APPLE_BUILD_ROOT}/apple-ios-simulator"

MACOS_UNIVERSAL_FRAMEWORK_PATH="${MACOS_UNIVERSAL_DIR}/${FRAMEWORK_NAME}.framework"
IOS_DEVICE_FRAMEWORK_PATH="${IOS_DEVICE_BUILD_DIR}/${FRAMEWORK_NAME}.framework"
IOS_SIM_FRAMEWORK_PATH="${IOS_SIM_BUILD_DIR}/${FRAMEWORK_NAME}.framework"

if [[ "${BUILD_SCOPE}" != "all" && "${BUILD_SCOPE}" != "ios" && "${BUILD_SCOPE}" != "osx" ]]; then
  echo "Unsupported build scope: ${BUILD_SCOPE}" >&2
  exit 1
fi

create_framework_bundle() {
  local dylib_path="$1"
  local framework_path="$2"
  local bundle_identifier="$3"

  rm -rf "${framework_path}"
  mkdir -p "${framework_path}/Headers" "${framework_path}/Modules"

  cp "${dylib_path}" "${framework_path}/${FRAMEWORK_NAME}"
  cp "${REPO_ROOT}/src/include/c_sweetline.h" "${framework_path}/Headers/c_sweetline.h"

  cat > "${framework_path}/Modules/module.modulemap" <<EOF
framework module ${FRAMEWORK_NAME} {
  umbrella header "c_sweetline.h"

  export *
}
EOF

  cat > "${framework_path}/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>${FRAMEWORK_NAME}</string>
  <key>CFBundleIdentifier</key>
  <string>${bundle_identifier}</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>${FRAMEWORK_NAME}</string>
  <key>CFBundlePackageType</key>
  <string>FMWK</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
  <key>CFBundleVersion</key>
  <string>1</string>
</dict>
</plist>
EOF

  install_name_tool -id "@rpath/${FRAMEWORK_NAME}.framework/${FRAMEWORK_NAME}" "${framework_path}/${FRAMEWORK_NAME}"
}

build_xcode_target() {
  local build_dir="$1"

  cmake --build "${build_dir}" --target "${TARGET_NAME}" --config Release -j 12 -- \
    CODE_SIGNING_ALLOWED=NO \
    CODE_SIGNING_REQUIRED=NO \
    CODE_SIGN_IDENTITY=
}

build_macos_arch() {
  local arch="$1"
  local build_dir="$2"

  rm -rf "${build_dir}"
  cmake -S "${REPO_ROOT}" -B "${build_dir}" \
    -G Xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_SYSROOT=macosx \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_OSX_ARCHITECTURES="${arch}" \
    -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIB=ON \
    -DBUILD_STATIC_LIB=OFF

  build_xcode_target "${build_dir}"
}

build_ios_variant() {
  local build_dir="$1"
  local sysroot="$2"
  local arch="$3"

  rm -rf "${build_dir}"
  cmake -S "${REPO_ROOT}" -B "${build_dir}" \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT="${sysroot}" \
    -DCMAKE_OSX_ARCHITECTURES="${arch}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIB=ON \
    -DBUILD_STATIC_LIB=OFF

  build_xcode_target "${build_dir}"
}

require_file() {
  local file_path="$1"
  local label="$2"

  if [[ ! -f "${file_path}" ]]; then
    echo "${label} not found at ${file_path}" >&2
    exit 1
  fi
}

build_macos_universal_framework() {
  local arm64_dylib=""
  local x64_dylib=""
  local universal_dylib="${MACOS_UNIVERSAL_DIR}/lib/libsweetline.dylib"

  build_macos_arch arm64 "${MACOS_ARM64_BUILD_DIR}"
  build_macos_arch x86_64 "${MACOS_X64_BUILD_DIR}"

  arm64_dylib="$(find_apple_dylib_path "${MACOS_ARM64_BUILD_DIR}")"
  x64_dylib="$(find_apple_dylib_path "${MACOS_X64_BUILD_DIR}")"

  require_file "${arm64_dylib}" "macOS arm64 shared library"
  require_file "${x64_dylib}" "macOS x86_64 shared library"

  rm -rf "${MACOS_UNIVERSAL_DIR}"
  mkdir -p "${MACOS_UNIVERSAL_DIR}/lib"
  lipo -create "${arm64_dylib}" "${x64_dylib}" -output "${universal_dylib}"

  create_framework_bundle "${universal_dylib}" "${MACOS_UNIVERSAL_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.macos"
}

build_ios_frameworks() {
  local ios_device_dylib=""
  local ios_sim_dylib=""

  build_ios_variant "${IOS_DEVICE_BUILD_DIR}" iphoneos arm64
  build_ios_variant "${IOS_SIM_BUILD_DIR}" iphonesimulator arm64

  ios_device_dylib="$(find_apple_dylib_path "${IOS_DEVICE_BUILD_DIR}")"
  ios_sim_dylib="$(find_apple_dylib_path "${IOS_SIM_BUILD_DIR}")"

  require_file "${ios_device_dylib}" "iOS device shared library"
  require_file "${ios_sim_dylib}" "iOS simulator shared library"

  create_framework_bundle "${ios_device_dylib}" "${IOS_DEVICE_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.ios"
  create_framework_bundle "${ios_sim_dylib}" "${IOS_SIM_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.ios-simulator"
}

create_xcframework() {
  local output_path="$1"
  shift
  local cmd=(xcodebuild -create-xcframework)
  local framework_path=""

  rm -rf "${output_path}"

  for framework_path in "$@"; do
    cmd+=( -framework "${framework_path}" )
  done
  cmd+=( -output "${output_path}" )

  "${cmd[@]}"
}

mkdir -p "${OUTPUT_DIR}"

case "${BUILD_SCOPE}" in
  osx)
    build_macos_universal_framework
    create_xcframework "${OUTPUT_XCFRAMEWORK}" "${MACOS_UNIVERSAL_FRAMEWORK_PATH}"
    echo "Generated ${OUTPUT_XCFRAMEWORK}"
    ;;
  ios)
    build_ios_frameworks
    create_xcframework "${OUTPUT_XCFRAMEWORK}" "${IOS_DEVICE_FRAMEWORK_PATH}" "${IOS_SIM_FRAMEWORK_PATH}"
    echo "Generated ${OUTPUT_XCFRAMEWORK}"
    ;;
  all)
    build_macos_universal_framework
    create_xcframework "${OUTPUT_DIR}/${APPLE_XCFRAMEWORK_OSX_NAME}" "${MACOS_UNIVERSAL_FRAMEWORK_PATH}"
    build_ios_frameworks
    create_xcframework "${OUTPUT_DIR}/${APPLE_XCFRAMEWORK_IOS_NAME}" "${IOS_DEVICE_FRAMEWORK_PATH}" "${IOS_SIM_FRAMEWORK_PATH}"
    echo "Generated ${OUTPUT_DIR}/${APPLE_XCFRAMEWORK_OSX_NAME}"
    echo "Generated ${OUTPUT_DIR}/${APPLE_XCFRAMEWORK_IOS_NAME}"
    ;;
esac
