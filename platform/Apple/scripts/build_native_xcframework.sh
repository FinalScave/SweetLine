#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPLE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${APPLE_DIR}/../.." && pwd)"
MACOS_BUILD_DIR="${REPO_ROOT}/build/apple-macos"
IOS_DEVICE_BUILD_DIR="${REPO_ROOT}/build/apple-ios-device"
IOS_SIM_BUILD_DIR="${REPO_ROOT}/build/apple-ios-simulator"
OUTPUT_DIR="${APPLE_DIR}/binaries"
OUTPUT_XCFRAMEWORK="${OUTPUT_DIR}/SweetLineNative.xcframework"
FRAMEWORK_NAME="SweetLineNative"
MACOS_LIB_PATH="${MACOS_BUILD_DIR}/lib/libsweetline.dylib"
IOS_DEVICE_LIB_PATH="${IOS_DEVICE_BUILD_DIR}/lib/Release/libsweetline.dylib"
IOS_SIM_LIB_PATH="${IOS_SIM_BUILD_DIR}/lib/Release/libsweetline.dylib"
MACOS_FRAMEWORK_PATH="${MACOS_BUILD_DIR}/${FRAMEWORK_NAME}.framework"
IOS_DEVICE_FRAMEWORK_PATH="${IOS_DEVICE_BUILD_DIR}/${FRAMEWORK_NAME}.framework"
IOS_SIM_FRAMEWORK_PATH="${IOS_SIM_BUILD_DIR}/${FRAMEWORK_NAME}.framework"

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

  cmake --build "${build_dir}" --config Release -- \
    CODE_SIGNING_ALLOWED=NO \
    CODE_SIGNING_REQUIRED=NO \
    CODE_SIGN_IDENTITY=
}

mkdir -p "${OUTPUT_DIR}"

cmake -S "${REPO_ROOT}" -B "${MACOS_BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
  -DBUILD_TESTING=OFF \
  -DBUILD_SHARED_LIB=ON \
  -DBUILD_STATIC_LIB=OFF

cmake --build "${MACOS_BUILD_DIR}" --config Release

cmake -S "${REPO_ROOT}" -B "${IOS_DEVICE_BUILD_DIR}" \
  -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DBUILD_TESTING=OFF \
  -DBUILD_SHARED_LIB=ON \
  -DBUILD_STATIC_LIB=OFF

build_xcode_target "${IOS_DEVICE_BUILD_DIR}"

cmake -S "${REPO_ROOT}" -B "${IOS_SIM_BUILD_DIR}" \
  -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DBUILD_TESTING=OFF \
  -DBUILD_SHARED_LIB=ON \
  -DBUILD_STATIC_LIB=OFF

build_xcode_target "${IOS_SIM_BUILD_DIR}"

if [[ ! -f "${MACOS_LIB_PATH}" ]]; then
  echo "Native macOS shared library not found at ${MACOS_LIB_PATH}" >&2
  exit 1
fi

if [[ ! -f "${IOS_DEVICE_LIB_PATH}" ]]; then
  echo "Native iOS device shared library not found at ${IOS_DEVICE_LIB_PATH}" >&2
  exit 1
fi

if [[ ! -f "${IOS_SIM_LIB_PATH}" ]]; then
  echo "Native iOS simulator shared library not found at ${IOS_SIM_LIB_PATH}" >&2
  exit 1
fi

create_framework_bundle "${MACOS_LIB_PATH}" "${MACOS_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.macos"
create_framework_bundle "${IOS_DEVICE_LIB_PATH}" "${IOS_DEVICE_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.ios"
create_framework_bundle "${IOS_SIM_LIB_PATH}" "${IOS_SIM_FRAMEWORK_PATH}" "com.qiplat.sweetline.native.ios-simulator"

rm -rf "${OUTPUT_XCFRAMEWORK}"
xcodebuild -create-xcframework \
  -framework "${MACOS_FRAMEWORK_PATH}" \
  -framework "${IOS_DEVICE_FRAMEWORK_PATH}" \
  -framework "${IOS_SIM_FRAMEWORK_PATH}" \
  -output "${OUTPUT_XCFRAMEWORK}"

echo "Generated ${OUTPUT_XCFRAMEWORK}"
