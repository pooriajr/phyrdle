#!/usr/bin/env bash

set -euo pipefail

# Script to compile and upload Arduino sketch
FQBN="arduino:avr:nano"
PORT="/dev/cu.usbserial-10"
NO_COMPILE=0
PORT_SET=0
for arg in "$@"; do
  case "$arg" in
    --no-compile) NO_COMPILE=1 ;;
    --help|-h)
      echo "Usage: ${0##*/} [PORT] [--no-compile]"
      echo "--no-compile uploads the last saved serial build."
      exit 0 ;;
    --*) echo "Unknown option: $arg" >&2; exit 2 ;;
    *)
      if [[ "$PORT_SET" == 1 ]]; then echo "Only one port may be specified." >&2; exit 2; fi
      PORT="$arg"; PORT_SET=1 ;;
  esac
done
SKETCH_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CPU_FREQUENCY="8000000L"

cd "$SKETCH_DIR"

BUILD_DIR="$SKETCH_DIR/build/upload-serial"
if [[ "$NO_COMPILE" == 0 ]]; then
  mkdir -p "$SKETCH_DIR/build"
  TEMP_BUILD="$(mktemp -d "$SKETCH_DIR/build/serial.XXXXXX")"
  trap 'rm -rf -- "$TEMP_BUILD"' EXIT
  echo "Compiling arduino_sketch.ino for $FQBN at 8 MHz..."
  arduino-cli compile --fqbn "$FQBN" \
    --build-property "build.f_cpu=$CPU_FREQUENCY" \
    --build-path "$TEMP_BUILD" arduino_sketch.ino
  rm -rf -- "$BUILD_DIR"
  mv -- "$TEMP_BUILD" "$BUILD_DIR"
elif [[ ! -f "$BUILD_DIR/arduino_sketch.ino.hex" ]]; then
  echo "No saved serial build. Run once without --no-compile first." >&2
  exit 1
else
  echo "Using last compiled serial build."
fi

echo "Uploading to $PORT..."
arduino-cli upload --port "$PORT" --fqbn "$FQBN" --input-dir "$BUILD_DIR" arduino_sketch.ino

echo "Upload successful!"
