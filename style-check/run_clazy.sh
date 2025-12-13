#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: style-check/run_clazy.sh [options] [files...]

Options:
  -b, --build-dir DIR   Build directory containing compile_commands.json (default: build)
  -c, --checks LIST     Comma-separated list of clazy checks (defaults to clazy level1)
  -h, --help            Show this message and exit

Any additional arguments are passed straight to clazy-standalone once the build
directory is prepared. If no files are provided (i.e. you only pass clazy
options), every translation unit listed in compile_commands.json is analyzed.
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="build"
CHECKS=""
FORWARDED_ARGS=()
USER_SPECIFIED_SOURCES=false

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
        --build-dir=*)
            BUILD_DIR="${1#*=}"
            shift
            ;;
        -c|--checks)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1" >&2
                exit 1
            fi
            CHECKS="$2"
            shift 2
            ;;
        --checks=*)
            CHECKS="${1#*=}"
            shift
            ;;
        -c=*)
            CHECKS="${1#*=}"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --export-fixes|--extra-arg|--extra-arg-before|--header-filter|--ignore-dirs|--vfsoverlay)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1" >&2
                exit 1
            fi
            FORWARDED_ARGS+=("$1" "$2")
            shift 2
            ;;
        --export-fixes=*|--extra-arg=*|--extra-arg-before=*|--header-filter=*|--ignore-dirs=*|--vfsoverlay=*)
            FORWARDED_ARGS+=("$1")
            shift
            ;;
        --)
            shift
            while [[ $# -gt 0 ]]; do
                FORWARDED_ARGS+=("$1")
                USER_SPECIFIED_SOURCES=true
                shift
            done
            break
            ;;
        --*)
            FORWARDED_ARGS+=("$1")
            shift
            ;;
        -*)
            FORWARDED_ARGS+=("$1")
            shift
            ;;
        *)
            FORWARDED_ARGS+=("$1")
            USER_SPECIFIED_SOURCES=true
            shift
            ;;
    esac
done

if [[ "$BUILD_DIR" != /* ]]; then
    BUILD_DIR="$ROOT_DIR/$BUILD_DIR"
fi

if ! command -v clazy-standalone >/dev/null 2>&1; then
    echo "clazy-standalone is not available in PATH." >&2
    exit 1
fi

echo "Configuring project in $BUILD_DIR..."
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null

COMPILE_COMMANDS="$BUILD_DIR/compile_commands.json"
if [[ ! -f "$COMPILE_COMMANDS" ]]; then
    echo "compile_commands.json was not generated in $BUILD_DIR" >&2
    exit 1
fi

if [[ "$USER_SPECIFIED_SOURCES" = false ]]; then
    AUTO_SOURCES=()
    mapfile -t AUTO_SOURCES < <(python3 - "$COMPILE_COMMANDS" <<'PY'
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

    if [[ ${#AUTO_SOURCES[@]} -eq 0 && ${#FORWARDED_ARGS[@]} -eq 0 ]]; then
        echo "No translation units found in compile_commands.json." >&2
        exit 0
    fi

    if [[ ${#AUTO_SOURCES[@]} -gt 0 ]]; then
        FORWARDED_ARGS+=("${AUTO_SOURCES[@]}")
    fi
fi

CMD=(clazy-standalone -p "$BUILD_DIR")
if [[ -n "$CHECKS" ]]; then
    CMD+=("--checks=$CHECKS")
fi
if [[ ${#FORWARDED_ARGS[@]} -eq 0 ]]; then
    echo "No translation units specified for analysis." >&2
    exit 0
fi

CMD+=("${FORWARDED_ARGS[@]}")

echo "Running: ${CMD[*]}"
"${CMD[@]}"
