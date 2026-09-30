#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STATE_FILE="$SKETCH_DIR/.usbasp-profile"
FQBN="arduino:avr:nano"
DIAGNOSTIC="${PHYRDLE_UPLOAD_DIAGNOSTIC:-0}"
EXTRA_FLAGS=""
if [[ "$DIAGNOSTIC" == 1 ]]; then
  STATE_FILE="$SKETCH_DIR/.usbasp-diagnostic-profile"
  EXTRA_FLAGS=" -DPHYRDLE_RESISTOR_DIAGNOSTIC=1"
fi

# Derive CLI names from filenames (yageo_10k.h -> yageo-10k).
profile_names=()
profile_files=()
for file in "$SKETCH_DIR"/profiles/*.h; do
  [[ -f "$file" ]] || continue
  filename="${file##*/}"
  stem="${filename%.h}"
  if [[ ! "$stem" =~ ^[A-Za-z0-9_][A-Za-z0-9_-]*$ ]]; then
    printf 'Unsupported profile filename: %s (use letters, numbers, underscores or hyphens).\n' "$filename" >&2
    exit 2
  fi
  name="${stem//_/-}"
  for existing in "${profile_names[@]-}"; do
    if [[ "$existing" == "$name" ]]; then
      printf 'Duplicate profile CLI name: %s\n' "$name" >&2; exit 2
    fi
  done
  profile_names+=("$name")
  profile_files+=("$filename")
done
if [[ ${#profile_names[@]} -eq 0 ]]; then
  printf 'No .h profiles found in %s/profiles.\n' "$SKETCH_DIR" >&2; exit 2
fi

usage() {
  printf 'Usage: %s [PROFILE | --last | --list]\n' "${0##*/}"
  printf 'Without arguments, choose a profile; Enter accepts the remembered choice.\n'
  printf 'Profiles discovered in profiles/:\n'
  printf '  %s\n' "${profile_names[@]}"
  printf 'Accepts a listed name, filename, or menu number. --last reuses the remembered profile.\n'
  printf 'Compiles at 8 MHz, uploads via USBasp, verifies, then remembers the selection.\n'
}

find_profile() {
  local input="${1%.h}" i
  input="${input//_/-}"
  for ((i=0; i<${#profile_names[@]}; i++)); do
    if [[ "$input" == "${profile_names[$i]}" || "$input" == "$((i+1))" ]]; then
      selected_index="$i"; return 0
    fi
  done
  return 1
}

if [[ $# -gt 1 ]]; then usage >&2; exit 2; fi
case "${1:-}" in
  --help|-h) usage; exit 0 ;;
  --list) printf '%s\n' "${profile_names[@]}"; exit 0 ;;
esac
previous="${profile_names[0]}"
if find_profile yageo-10k; then previous=yageo-10k; fi
if [[ -f "$STATE_FILE" ]]; then
  saved="$(cat "$STATE_FILE")"
  if find_profile "$saved"; then
    previous="${profile_names[$selected_index]}"
  else
    printf 'Remembered profile %s is missing. Choose an available profile.\n' "$saved" >&2
    if [[ "${1:-}" == --last ]]; then exit 2; fi
  fi
fi

if [[ $# -eq 0 ]]; then
  printf 'Hardware profiles:\n'
  for ((i=0; i<${#profile_names[@]}; i++)); do
    printf '  %s) %s\n' "$((i+1))" "${profile_names[$i]}"
  done
  printf 'Choose number or name [%s]: ' "$previous"
  if ! IFS= read -r choice; then
    printf '\nNo selection received. Use --last or pass a profile name.\n' >&2
    exit 2
  fi
  choice="${choice:-$previous}"
elif [[ "$1" == --last ]]; then
  choice="$previous"
else
  choice="$1"
fi
if ! find_profile "$choice"; then
  printf 'Unknown profile: %s\n' "$choice" >&2
  usage >&2; exit 2
fi
choice="${profile_names[$selected_index]}"
profile_file="${profile_files[$selected_index]}"
command -v arduino-cli >/dev/null || { printf 'arduino-cli is required on PATH.\n' >&2; exit 1; }

BUILD_DIR="$(mktemp -d "${TMPDIR:-/tmp}/tactle-usbasp.XXXXXX")"
trap 'rm -rf -- "$BUILD_DIR"' EXIT
if [[ "$DIAGNOSTIC" == 1 ]]; then printf 'Mode: resistor diagnostic\n'; fi
printf 'Compiling %s for %s at 8 MHz...\n' "$choice" "$FQBN"
arduino-cli compile --fqbn "$FQBN" \
  --build-property build.f_cpu=8000000L \
  --build-property "build.extra_flags=-DPHYRDLE_PROFILE_FILE=profiles/$profile_file$EXTRA_FLAGS" \
  --build-path "$BUILD_DIR" "$SKETCH_DIR"
printf 'Uploading %s via USBasp and verifying...\n' "$choice"
arduino-cli upload --fqbn "$FQBN" --programmer usbasp \
  --input-dir "$BUILD_DIR" --verify "$SKETCH_DIR"
printf '%s\n' "$choice" > "$STATE_FILE"
printf 'Upload verified. Remembered profile: %s\n' "$choice"
