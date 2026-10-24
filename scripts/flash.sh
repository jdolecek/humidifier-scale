#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [ "$#" -ne 1 ]; then
  echo "Usage: bash scripts/flash.sh /dev/cu.usbserial-10" >&2
  exit 1
fi
bash scripts/build.sh
python_options=()
if [ -x .venv/bin/python3 ]; then
  python_options=(--upload-property "runtime.tools.python3.path=$PWD/.venv/bin")
fi
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port "$1" --input-dir build "${python_options[@]}" .
