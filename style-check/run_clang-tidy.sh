#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: style-check/run_clang-tidy.sh [-b build_dir] [clang-tidy args...]

Examples:
  style-check/run_clang-tidy.sh                    # analyze every TU in compile_commands.json
  style-check/run_clang-tidy.sh src/Foo.cpp        # analyze only Foo.cpp
  style-check/run_clang-tidy.sh -b out Debug/*.cpp # custom build directory
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="build"
FORWARDED_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--build-dir)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1" >&2
                exit 1
            fi
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            FORWARDED_ARGS+=("$@")
            break
            ;;
        *)
            FORWARDED_ARGS+=("$1")
            shift
            ;;
    esac
done

if [[ "$BUILD_DIR" != /* ]]; then
    BUILD_DIR="$ROOT_DIR/$BUILD_DIR"
fi

echo "Configuring project in $BUILD_DIR..."
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null

COMPILE_COMMANDS="$BUILD_DIR/compile_commands.json"
if [[ ! -f "$COMPILE_COMMANDS" ]]; then
    echo "compile_commands.json was not generated in $BUILD_DIR" >&2
    exit 1
fi

if command -v run-clang-tidy >/dev/null 2>&1; then
    echo "Running run-clang-tidy..."
    if [[ ${#FORWARDED_ARGS[@]} -gt 0 ]]; then
        run-clang-tidy -p "$BUILD_DIR" "${FORWARDED_ARGS[@]}"
    else
        run-clang-tidy -p "$BUILD_DIR"
    fi
    exit $?
fi

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "Neither run-clang-tidy nor clang-tidy is available in PATH" >&2
    exit 1
fi

if [[ ${#FORWARDED_ARGS[@]} -eq 0 ]]; then
    mapfile -t FORWARDED_ARGS < <(python3 - "$COMPILE_COMMANDS" <<'PY'
import json
import sys

path = sys.argv[1]
with open(path, encoding='utf-8') as fp:
    data = json.load(fp)

seen = set()
for entry in data:
    filename = entry.get("file")
    if not filename or filename in seen:
        continue
    seen.add(filename)
    print(filename)
PY
)
fi

if [[ ${#FORWARDED_ARGS[@]} -eq 0 ]]; then
    echo "No translation units to analyze." >&2
    exit 0
fi

echo "run-clang-tidy not found, falling back to invoking clang-tidy per file."
for source in "${FORWARDED_ARGS[@]}"; do
    echo "Analyzing $source"
    clang-tidy "$source" -p "$BUILD_DIR"
done
