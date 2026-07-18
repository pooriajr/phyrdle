#!/usr/bin/env bash

set -euo pipefail

# Script to compile and upload Arduino sketch
FQBN="arduino:avr:nano"
PORT="${1:-/dev/cu.usbserial-10}"
SKETCH_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

cd "$SKETCH_DIR"

echo "Compiling arduino_sketch.ino for $FQBN..."
arduino-cli compile --fqbn "$FQBN" arduino_sketch.ino

echo "Uploading to $PORT..."
arduino-cli upload --port "$PORT" --fqbn "$FQBN" arduino_sketch.ino

echo "Upload successful!"
