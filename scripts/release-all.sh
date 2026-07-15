#!/bin/bash

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
KMP_DIR="$PROJECT_DIR/platform/KMP"
IOS_DIR="$PROJECT_DIR/platform/iOS"
MACOS_DIR="$PROJECT_DIR/platform/macOS"
TARGET_ORDER=("kmp" "ios" "macos")
SELECTED_TARGETS=()

fail() {
  echo "$1" >&2
  return 1
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || fail "Required command is unavailable: $1"
}

run_external() {
  local working_dir="$1"
  shift
  printf '\n> '
  printf '%q ' "$@"
  printf '\n'
  (cd "$working_dir" && "$@")
}

target_label() {
  case "$1" in
    kmp) echo "Kotlin Multiplatform" ;;
    ios) echo "iOS / SwiftPM" ;;
    macos) echo "macOS / SwiftPM" ;;
    *) return 1 ;;
  esac
}

target_registry() {
  case "$1" in
    kmp) echo "Maven Central" ;;
    ios|macos) echo "GitHub" ;;
    *) return 1 ;;
  esac
}

target_path() {
  case "$1" in
    kmp) echo "$KMP_DIR" ;;
    ios) echo "$IOS_DIR" ;;
    macos) echo "$MACOS_DIR" ;;
    *) return 1 ;;
  esac
}

target_repository() {
  case "$1" in
    ios) echo "Xiue233/SweetLine-iOS" ;;
    macos) echo "Xiue233/SweetLine-macOS" ;;
    *) return 1 ;;
  esac
}

is_target() {
  case "$1" in
    kmp|ios|macos) return 0 ;;
    *) return 1 ;;
  esac
}

get_kmp_version() {
  sed -nE 's/^[[:space:]]*version[[:space:]]*=[[:space:]]*"([^"]+)"[[:space:]]*$/\1/p' "$KMP_DIR/sweetline/build.gradle.kts" | head -n 1
}

get_apple_version() {
  local package_path="$(target_path "$1")/Package.swift"
  if [ ! -f "$package_path" ]; then
    return 0
  fi
  sed -nE 's#.*releases/download/v([^/]+)/.*#\1#p' "$package_path" | head -n 1
}

target_version() {
  case "$1" in
    kmp) get_kmp_version ;;
    ios|macos) get_apple_version "$1" ;;
    *) return 1 ;;
  esac
}

assert_version() {
  local target="$1"
  local version="$2"
  if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+([-+][0-9A-Za-z.-]+)?$ ]]; then
    fail "Invalid version for $target: $version"
  fi
}

set_kmp_version() {
  local new_version="$1"
  local build_file="$KMP_DIR/sweetline/build.gradle.kts"
  local readme_file="$KMP_DIR/sweetline/README.md"
  local build_tmp
  local readme_tmp
  local current_version

  assert_version "kmp" "$new_version" || return 1
  current_version="$(get_kmp_version)"
  if [ "$current_version" = "$new_version" ]; then
    echo "Kotlin Multiplatform is already $new_version."
    return 0
  fi
  if [ "$(grep -Ec '^[[:space:]]*version[[:space:]]*=[[:space:]]*"[^"]+"[[:space:]]*$' "$build_file")" -ne 1 ]; then
    fail "Unable to locate the KMP Gradle version."
    return 1
  fi
  if [ "$(grep -Ec '^[[:space:]]*implementation\("com\.qiplat:sweetline-kmp:[^"]+"\)[[:space:]]*$' "$readme_file")" -ne 1 ]; then
    fail "Unable to locate the KMP README dependency version."
    return 1
  fi

  build_tmp="$(mktemp "${TMPDIR:-/tmp}/sweetline-kmp-build.XXXXXX")" || return 1
  readme_tmp="$(mktemp "${TMPDIR:-/tmp}/sweetline-kmp-readme.XXXXXX")" || {
    rm -f "$build_tmp"
    return 1
  }
  sed -E "s/^[[:space:]]*version[[:space:]]*=[[:space:]]*\"[^\"]+\"[[:space:]]*$/version = \"$new_version\"/" "$build_file" > "$build_tmp" || {
    rm -f "$build_tmp" "$readme_tmp"
    return 1
  }
  sed -E "s#^[[:space:]]*implementation\\(\"com\\.qiplat:sweetline-kmp:[^\"]+\"\\)[[:space:]]*\$#implementation(\"com.qiplat:sweetline-kmp:$new_version\")#" "$readme_file" > "$readme_tmp" || {
    rm -f "$build_tmp" "$readme_tmp"
    return 1
  }
  cp "$build_tmp" "$build_file" && cp "$readme_tmp" "$readme_file"
  local result=$?
  rm -f "$build_tmp" "$readme_tmp"
  if [ "$result" -ne 0 ]; then
    return "$result"
  fi
  echo "Kotlin Multiplatform version updated: $current_version -> $new_version"
}

registry_status() {
  local target="$1"
  local version="$2"
  local code
  local response_file

  if [ -z "$version" ]; then
    if [ "$target" = "ios" ] || [ "$target" = "macos" ]; then
      echo "子仓库未初始化"
    else
      echo "需要版本"
    fi
    return 0
  fi
  if [ "$target" = "kmp" ]; then
    response_file="$(mktemp "${TMPDIR:-/tmp}/sweetline-maven-status.XXXXXX")" || {
      echo "查询失败"
      return 0
    }
    code="$(curl -sS -o "$response_file" -w '%{http_code}' "https://repo.maven.apache.org/maven2/com/qiplat/sweetline-kmp/maven-metadata.xml" 2>/dev/null)"
    case "$code" in
      200)
        if grep -Fq "<version>$version</version>" "$response_file"; then
          echo "已发布"
        else
          echo "待发布"
        fi
        ;;
      404) echo "待发布" ;;
      *) echo "查询失败" ;;
    esac
    rm -f "$response_file"
    return 0
  fi

  code="$(curl -sS -o /dev/null -w '%{http_code}' "https://api.github.com/repos/$(target_repository "$target")/releases/tags/v$version" 2>/dev/null)"
  case "$code" in
    200) echo "已发布" ;;
    404) echo "待发布" ;;
    *) echo "查询失败" ;;
  esac
}

show_status() {
  local target="$1"
  local version
  local status
  version="$(target_version "$target")"
  status="$(registry_status "$target" "$version")"
  printf '\n%-8s %-12s %-16s %s\n' "Target" "Version" "Registry" "Status"
  printf '%-8s %-12s %-16s %s\n' "$target" "${version:--}" "$(target_registry "$target")" "$status"
}

resolve_llvm_tool() {
  local name="$1"
  local path
  path="$(command -v "$name" 2>/dev/null || true)"
  if [ -n "$path" ]; then
    echo "$path"
    return 0
  fi
  if command -v xcrun >/dev/null 2>&1; then
    path="$(xcrun --find "$name" 2>/dev/null || true)"
    if [ -n "$path" ]; then
      echo "$path"
      return 0
    fi
  fi
  return 1
}

test_kmp_native_abi() {
  local required_files=(
    "$PROJECT_DIR/prebuilt/windows/x64/sweetline.dll"
    "$PROJECT_DIR/prebuilt/linux/x86_64/libsweetline.so"
    "$PROJECT_DIR/prebuilt/linux/aarch64/libsweetline.so"
    "$PROJECT_DIR/prebuilt/osx/x86_64/libsweetline.dylib"
    "$PROJECT_DIR/prebuilt/osx/arm64/libsweetline.dylib"
    "$PROJECT_DIR/prebuilt/ios/arm64/libsweetline.dylib"
    "$PROJECT_DIR/prebuilt/ios/simulator-arm64/libsweetline.dylib"
  )
  local file
  local llvm_nm
  local llvm_readobj
  local expected_file
  local exports_file
  local missing_file
  local failures=0

  for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
      echo "Missing KMP native artifact: $file" >&2
      failures=1
    fi
  done
  if [ "$failures" -ne 0 ]; then
    return 1
  fi

  llvm_nm="$(resolve_llvm_tool llvm-nm)" || {
    fail "Required command is unavailable: llvm-nm"
    return 1
  }
  llvm_readobj="$(resolve_llvm_tool llvm-readobj)" || {
    fail "Required command is unavailable: llvm-readobj"
    return 1
  }
  expected_file="$(mktemp "${TMPDIR:-/tmp}/sweetline-symbols.XXXXXX")" || return 1
  exports_file="$(mktemp "${TMPDIR:-/tmp}/sweetline-exports.XXXXXX")" || {
    rm -f "$expected_file"
    return 1
  }
  missing_file="$(mktemp "${TMPDIR:-/tmp}/sweetline-missing.XXXXXX")" || {
    rm -f "$expected_file" "$exports_file"
    return 1
  }

  grep -Eo 'sl_[a-z0-9_]+[[:space:]]*\(' "$PROJECT_DIR/include/sweetline/c_sweetline.h" \
    | sed -E 's/[[:space:]]*\($//' \
    | sort -u > "$expected_file"

  for file in "${required_files[@]}"; do
    if [[ "$file" = *.dll ]]; then
      "$llvm_readobj" --coff-exports "$file" 2>/dev/null \
        | sed -nE 's/.*Name:[[:space:]]+(sl_[a-z0-9_]+).*/\1/p' \
        | sort -u > "$exports_file"
    elif [[ "$file" = *.so ]]; then
      "$llvm_nm" -D --defined-only "$file" 2>/dev/null \
        | awk '{ name=$NF; sub(/^_/, "", name); if (name ~ /^sl_[a-z0-9_]+$/) print name }' \
        | sort -u > "$exports_file"
    else
      "$llvm_nm" --defined-only "$file" 2>/dev/null \
        | awk '{ name=$NF; sub(/^_/, "", name); if (name ~ /^sl_[a-z0-9_]+$/) print name }' \
        | sort -u > "$exports_file"
    fi
    comm -23 "$expected_file" "$exports_file" > "$missing_file"
    if [ -s "$missing_file" ]; then
      echo "Native ABI validation failed: ${file#$PROJECT_DIR/}" >&2
      paste -sd ', ' "$missing_file" >&2
      failures=1
    fi
  done

  rm -f "$expected_file" "$exports_file" "$missing_file"
  if [ "$failures" -ne 0 ]; then
    return 1
  fi
  echo "Native ABI validation passed."
}

apple_archive_path() {
  case "$1" in
    ios) echo "$PROJECT_DIR/prebuilt/ios/SweetLineCoreIOS.xcframework.zip" ;;
    macos) echo "$PROJECT_DIR/prebuilt/osx/SweetLineCoreOSX.xcframework.zip" ;;
    *) return 1 ;;
  esac
}

test_apple_artifact() {
  local target="$1"
  local version="$2"
  local target_dir
  local package_path
  local archive_path
  local expected_checksum
  local actual_checksum
  local package_version

  target_dir="$(target_path "$target")"
  package_path="$target_dir/Package.swift"
  archive_path="$(apple_archive_path "$target")"
  if [ ! -f "$package_path" ]; then
    fail "$(target_label "$target") submodule is not initialized."
    return 1
  fi
  if [ ! -f "$archive_path" ]; then
    fail "Missing Apple artifact: $archive_path"
    return 1
  fi
  require_command swift || return 1
  expected_checksum="$(sed -nE 's/.*checksum:[[:space:]]*"([0-9a-fA-F]{64})".*/\1/p' "$package_path" | head -n 1)"
  if [ -z "$expected_checksum" ]; then
    fail "Unable to read binary target checksum from $package_path"
    return 1
  fi
  actual_checksum="$(swift package compute-checksum "$archive_path")" || return 1
  actual_checksum="$(printf '%s' "$actual_checksum" | tr '[:upper:]' '[:lower:]')"
  expected_checksum="$(printf '%s' "$expected_checksum" | tr '[:upper:]' '[:lower:]')"
  if [ "$actual_checksum" != "$expected_checksum" ]; then
    fail "$(target_label "$target") binary target checksum does not match the prebuilt archive."
    return 1
  fi
  package_version="$(get_apple_version "$target")"
  if [ "$package_version" != "$version" ]; then
    fail "$(target_label "$target") Package.swift references v$package_version instead of v$version."
    return 1
  fi
  run_external "$target_dir" bash Scripts/verify-package.sh
}

package_target() {
  local target="$1"
  local version="$2"
  printf '\nPackaging %s %s\n' "$(target_label "$target")" "$version"
  case "$target" in
    kmp)
      test_kmp_native_abi || return 1
      run_external "$KMP_DIR" ./gradlew :sweetline:publishToMavenLocal --no-configuration-cache --console=plain
      ;;
    ios|macos)
      test_apple_artifact "$target" "$version"
      ;;
  esac
}

assert_publish_workspace() {
  local changes
  local head
  local upstream
  changes="$(git -C "$PROJECT_DIR" status --porcelain)" || return 1
  if [ -n "$changes" ]; then
    fail "Publishing requires a clean working tree."
    return 1
  fi
  head="$(git -C "$PROJECT_DIR" rev-parse HEAD)" || return 1
  upstream="$(git -C "$PROJECT_DIR" rev-parse '@{u}')" || return 1
  if [ "$head" != "$upstream" ]; then
    fail "Publishing requires HEAD to match its upstream branch."
  fi
}

assert_submodule_publish_state() {
  local target="$1"
  local target_dir
  local changes
  local head
  local upstream
  target_dir="$(target_path "$target")"
  if [ ! -f "$target_dir/Package.swift" ]; then
    fail "$(target_label "$target") submodule is not initialized."
    return 1
  fi
  changes="$(git -C "$target_dir" status --porcelain)" || return 1
  if [ -n "$changes" ]; then
    fail "$(target_label "$target") submodule has uncommitted changes."
    return 1
  fi
  head="$(git -C "$target_dir" rev-parse HEAD)" || return 1
  upstream="$(git -C "$target_dir" rev-parse '@{u}')" || return 1
  if [ "$head" != "$upstream" ]; then
    fail "$(target_label "$target") submodule HEAD is not pushed."
    return 1
  fi
  echo "$head"
}

publish_target() {
  local target="$1"
  local version="$2"
  local head
  local archive_path

  printf '\nPublishing %s %s\n' "$(target_label "$target")" "$version"
  case "$target" in
    kmp)
      run_external "$KMP_DIR" ./gradlew :sweetline:publishAndReleaseToMavenCentral --no-configuration-cache --console=plain
      ;;
    ios|macos)
      head="$(assert_submodule_publish_state "$target")" || return 1
      archive_path="$(apple_archive_path "$target")"
      run_external "$(target_path "$target")" gh release create "v$version" "$archive_path" \
        --repo "$(target_repository "$target")" \
        --title "$(target_label "$target") v$version" \
        --generate-notes \
        --target "$head"
      ;;
  esac
}

resolve_selected_versions() {
  RESOLVED_VERSIONS=()
  local target
  local version
  for target in "${SELECTED_TARGETS[@]}"; do
    version="$(target_version "$target")"
    if [ -z "$version" ]; then
      fail "$(target_label "$target") has no readable version."
      return 1
    fi
    assert_version "$target" "$version" || return 1
    RESOLVED_VERSIONS+=("$version")
  done
}

package_selected() {
  local index
  resolve_selected_versions || return 1
  for ((index=0; index<${#SELECTED_TARGETS[@]}; index++)); do
    package_target "${SELECTED_TARGETS[$index]}" "${RESOLVED_VERSIONS[$index]}" || return 1
  done
  echo
  echo "All selected packages passed validation."
}

publish_selected() {
  local pending_targets=()
  local pending_versions=()
  local index
  local target
  local version
  local status
  local needs_gpg=0
  local needs_gh=0

  resolve_selected_versions || return 1
  printf '\n%-8s %-12s %-16s %s\n' "Target" "Version" "Registry" "Status"
  for ((index=0; index<${#SELECTED_TARGETS[@]}; index++)); do
    target="${SELECTED_TARGETS[$index]}"
    version="${RESOLVED_VERSIONS[$index]}"
    status="$(registry_status "$target" "$version")"
    printf '%-8s %-12s %-16s %s\n' "$target" "$version" "$(target_registry "$target")" "$status"
    case "$status" in
      已发布) ;;
      待发布)
        pending_targets+=("$target")
        pending_versions+=("$version")
        ;;
      *)
        fail "Unable to safely publish $target: $status"
        return 1
        ;;
    esac
  done
  if [ "${#pending_targets[@]}" -eq 0 ]; then
    echo "All selected versions are already published."
    return 0
  fi

  assert_publish_workspace || return 1
  for target in "${pending_targets[@]}"; do
    if [ "$target" = "kmp" ]; then
      needs_gpg=1
    else
      needs_gh=1
    fi
  done
  if [ "$needs_gpg" -eq 1 ]; then
    require_command gpg || return 1
  fi
  if [ "$needs_gh" -eq 1 ]; then
    require_command gh || return 1
    run_external "$PROJECT_DIR" gh auth status || return 1
  fi

  for ((index=0; index<${#pending_targets[@]}; index++)); do
    package_target "${pending_targets[$index]}" "${pending_versions[$index]}" || {
      fail "Packaging failed. Nothing has been published."
      return 1
    }
  done
  for ((index=0; index<${#pending_targets[@]}; index++)); do
    publish_target "${pending_targets[$index]}" "${pending_versions[$index]}" || {
      fail "Publishing stopped at ${pending_targets[$index]}. Re-run the same selection after resolving the problem."
      return 1
    }
  done
  echo
  echo "All selected platforms were published."
}

print_target_menu() {
  local target
  local version
  local index=1
  echo
  for target in "${TARGET_ORDER[@]}"; do
    version="$(target_version "$target")"
    printf '%2d. %-28s %s\n' "$index" "$(target_label "$target")" "${version:-需要初始化子仓库}"
    index=$((index + 1))
  done
  echo " 0. 取消"
}

read_single_target() {
  local only_editable="${1:-0}"
  local selection
  if [ "$only_editable" -eq 1 ]; then
    printf '\n 1. %-28s %s\n' "$(target_label kmp)" "$(target_version kmp)" >&2
    echo " 0. 取消" >&2
    read -r -p "请选择要修改版本的平台: " selection
    case "$selection" in
      1) echo "kmp" ;;
      0|"") return 1 ;;
      *) fail "Invalid target selection: $selection" ;;
    esac
    return
  fi

  print_target_menu >&2
  read -r -p "请选择平台: " selection
  case "$selection" in
    1) echo "kmp" ;;
    2) echo "ios" ;;
    3) echo "macos" ;;
    0|"") return 1 ;;
    *) fail "Invalid target selection: $selection" ;;
  esac
}

read_target_selection() {
  local selection
  local part
  local target
  local parts=()
  SELECTED_TARGETS=()
  print_target_menu
  read -r -p "请选择平台，可用逗号分隔，输入 all 选择全部: " selection
  if [ "$selection" = "0" ] || [ -z "$selection" ]; then
    return 1
  fi
  if [ "$selection" = "all" ]; then
    SELECTED_TARGETS=("${TARGET_ORDER[@]}")
    return 0
  fi
  IFS=',' read -r -a parts <<< "$selection"
  for part in "${parts[@]}"; do
    part="$(printf '%s' "$part" | tr -d '[:space:]')"
    case "$part" in
      1) target="kmp" ;;
      2) target="ios" ;;
      3) target="macos" ;;
      *) fail "Invalid target selection: $part"; return 1 ;;
    esac
    if [[ " ${SELECTED_TARGETS[*]} " != *" $target "* ]]; then
      SELECTED_TARGETS+=("$target")
    fi
  done
}

show_main_menu() {
  local choice
  local selected
  local new_version
  while true; do
    printf '\nSweetLine macOS 发布工具\n\n'
    echo "1. 查看单个平台状态"
    echo "2. 修改 KMP 版本"
    echo "3. 校验并打包"
    echo "4. 发布选定平台"
    echo "0. 退出"
    read -r -p "请选择操作: " choice
    case "$choice" in
      1)
        selected="$(read_single_target)" || continue
        show_status "$selected" || true
        ;;
      2)
        selected="$(read_single_target 1)" || continue
        read -r -p "当前版本 $(target_version "$selected")，请输入新版本: " new_version
        if [ -n "$new_version" ]; then
          set_kmp_version "$new_version" || true
        fi
        ;;
      3)
        read_target_selection || continue
        package_selected || true
        ;;
      4)
        read_target_selection || continue
        publish_selected || true
        ;;
      0) return 0 ;;
      *) echo "无效选项。" >&2 ;;
    esac
  done
}

expand_cli_targets() {
  local argument
  local part
  local parts=()
  SELECTED_TARGETS=()
  for argument in "$@"; do
    if [ "$argument" = "all" ]; then
      SELECTED_TARGETS=("${TARGET_ORDER[@]}")
      return 0
    fi
    IFS=',' read -r -a parts <<< "$argument"
    for part in "${parts[@]}"; do
      is_target "$part" || {
        fail "Unknown release target: $part"
        return 1
      }
      if [[ " ${SELECTED_TARGETS[*]} " != *" $part "* ]]; then
        SELECTED_TARGETS+=("$part")
      fi
    done
  done
}

show_usage() {
  cat <<'EOF'
Usage:
  ./scripts/release-all.sh
  ./scripts/release-all.sh status <kmp|ios|macos>
  ./scripts/release-all.sh package [all|targets...]
  ./scripts/release-all.sh publish <targets...>
EOF
}

main() {
  local action="${1:-menu}"
  if [ "$action" = "-h" ] || [ "$action" = "--help" ]; then
    show_usage
    return 0
  fi
  if [ "$(uname -s)" != "Darwin" ]; then
    fail "release-all.sh only supports macOS. Use release-all.ps1 on Windows."
    return 1
  fi
  require_command curl || return 1
  require_command git || return 1

  if [ "$#" -gt 0 ]; then
    shift
  fi
  case "$action" in
    menu) show_main_menu ;;
    status)
      if [ "$#" -ne 1 ]; then
        fail "The status action requires exactly one target."
        return 1
      fi
      is_target "$1" || {
        fail "Unknown release target: $1"
        return 1
      }
      show_status "$1"
      ;;
    package)
      if [ "$#" -eq 0 ]; then
        SELECTED_TARGETS=("${TARGET_ORDER[@]}")
      else
        expand_cli_targets "$@" || return 1
      fi
      package_selected
      ;;
    publish)
      if [ "$#" -eq 0 ]; then
        fail "The publish action requires at least one target."
        return 1
      fi
      expand_cli_targets "$@" || return 1
      publish_selected
      ;;
    *)
      fail "Unknown action: $action"
      show_usage
      return 1
      ;;
  esac
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  main "$@"
fi
