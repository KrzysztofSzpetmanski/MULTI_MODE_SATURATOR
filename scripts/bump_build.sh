#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PLUGIN_JSON="$ROOT_DIR/plugin.json"
HEADER_FILE="$ROOT_DIR/src/platform/vcv/DualFilterModule.h"

current_version="$(sed -nE 's/^[[:space:]]*"version"[[:space:]]*:[[:space:]]*"([^"]+)".*/\1/p' "$PLUGIN_JSON" | head -n 1)"
next_build=1
if [[ "$current_version" =~ ^2\.0\.([0-9]{3})$ ]]; then
	curr_build="${BASH_REMATCH[1]}"
	next_build=$((10#$curr_build + 1))
fi

next_build_padded="$(printf '%03d' "$next_build")"
next_version="2.0.${next_build_padded}"

perl -0777 -i -pe "s/\"version\"\s*:\s*\"[^\"]+\"/\"version\": \"${next_version}\"/" "$PLUGIN_JSON"
perl -0777 -i -pe "s/static constexpr int kBuildNumber = \d+;/static constexpr int kBuildNumber = ${next_build};/" "$HEADER_FILE"

echo "[bump-build] version=${next_version} build=${next_build}"
