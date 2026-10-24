#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
arduino-cli compile --fqbn esp8266:esp8266:d1_mini --build-path build/work --output-dir build .
