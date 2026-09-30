#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ "${1:-}" == --help || "${1:-}" == -h ]]; then
  printf 'Resistor diagnostic uploader (USBasp, Nano 8 MHz)\n'
  printf 'Usage: ./upload_diagnostic_usbasp.sh [PROFILE | --last | --list]\n'
  printf 'Profiles are discovered automatically from profiles/*.h; use --list to see them.\n'
  printf 'No arguments: profile picker; Enter recalls the last successful diagnostic profile.\n'
  printf 'Red: range changes/invalid. Yellow: near edge. Green: central 50%%.\n'
  printf 'Dim white: empty. Blue: collecting the initial 16 averaged samples.\n'
  exit 0
fi
export PHYRDLE_UPLOAD_DIAGNOSTIC=1
exec "$SCRIPT_DIR/upload_usbasp.sh" "$@"
