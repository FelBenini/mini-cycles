#!/usr/bin/env bash
set -e

# Usage:
#   ./denoise.sh input.png
#   ./denoise.sh input.png output.png
#   ./denoise.sh input.png output.png --hdr   (for HDR/linear renders)

INPUT="$1"

if [[ -z "$INPUT" ]]; then
  echo "Usage: $0 <input.png> [output.png] [--hdr]"
  exit 1
fi

ABS_INPUT=$(realpath "$INPUT")
MOUNT_DIR=$(dirname "$ABS_INPUT")
BASE=$(basename "$ABS_INPUT")
NAME="${BASE%.*}"
EXT="${BASE##*.}"

# Parse optional output path and extra flags
shift
OUTPUT=""
EXTRA_FLAGS=""
for arg in "$@"; do
  if [[ "$arg" == --* ]]; then
    EXTRA_FLAGS="$EXTRA_FLAGS $arg"
  else
    OUTPUT="$arg"
  fi
done

# Default output name
if [[ -z "$OUTPUT" ]]; then
  OUTPUT="$MOUNT_DIR/denoised_${NAME}.${EXT}"
else
  OUTPUT=$(realpath -m "$OUTPUT")
  # If output is in a different dir, warn and redirect into same dir as input
  if [[ "$(dirname "$OUTPUT")" != "$MOUNT_DIR" ]]; then
    echo "\u26a0 Output must be in the same directory as input. Redirecting to: $MOUNT_DIR/$(basename "$OUTPUT")"
    OUTPUT="$MOUNT_DIR/$(basename "$OUTPUT")"
  fi
fi

echo "\u2192 Input:  $ABS_INPUT"
echo "\u2192 Output: $OUTPUT"

# oidnDenoise uses --ldr for sRGB PNGs, --hdr for linear/HDR images
if echo "$EXTRA_FLAGS" | grep -q "\-\-hdr"; then
  INPUT_FLAG="--hdr"
  EXTRA_FLAGS=$(echo "$EXTRA_FLAGS" | sed 's/--hdr//')
else
  INPUT_FLAG="--ldr"
fi

docker run --rm \
  -v "$MOUNT_DIR":/images \
  oidn \
  $INPUT_FLAG "/images/$BASE" \
  --output "/images/$(basename "$OUTPUT")" \
  $EXTRA_FLAGS

echo "Done: $OUTPUT"
