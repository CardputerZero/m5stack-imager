#!/bin/bash
#
# Regenerate Windows .ico file from PNG sources
# Requires ImageMagick (magick or convert command)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ICON_DIR="${SCRIPT_DIR}/icon"
OUTPUT_ICO="${SCRIPT_DIR}/../icons/rpi-imager.ico"

# Check for ImageMagick
if command -v magick &> /dev/null; then
    CONVERT_CMD="magick"
elif command -v convert &> /dev/null; then
    CONVERT_CMD="convert"
else
    CONVERT_CMD=""
fi

# Windows .ico supports these sizes (in order of preference for Windows)
# Using the available sizes from the icon directory
SIZES=(16 20 24 32 40 48 64 256)

echo "Generating Windows icon from PNGs..."
if [[ -n "$CONVERT_CMD" ]]; then
    echo "Using ImageMagick: $CONVERT_CMD"
else
    echo "Using Python ICO writer fallback"
fi
echo ""

# Build list of input files (only use sizes that Windows actually uses)
INPUT_FILES=""
for size in "${SIZES[@]}"; do
    PNG_FILE="${ICON_DIR}/Windows imager icon Full hight_WIN ${size}x${size}.png"
    if [[ -f "$PNG_FILE" ]]; then
        INPUT_FILES="$INPUT_FILES \"$PNG_FILE\""
        echo "  Adding ${size}x${size}"
    else
        echo "  Warning: ${size}x${size} not found, skipping"
    fi
done

if [[ -z "$INPUT_FILES" ]]; then
    echo "Error: No input PNG files found in ${ICON_DIR}"
    exit 1
fi

echo ""
echo "Creating ${OUTPUT_ICO}..."

if [[ -n "$CONVERT_CMD" ]]; then
    eval "$CONVERT_CMD" $INPUT_FILES "${OUTPUT_ICO}"
else
    python3 - "$ICON_DIR" "$OUTPUT_ICO" "${SIZES[@]}" <<'PY'
import struct
import sys
from pathlib import Path

icon_dir = Path(sys.argv[1])
output_ico = Path(sys.argv[2])
sizes = [int(v) for v in sys.argv[3:]]
images = []

for size in sizes:
    png_file = icon_dir / f"Windows imager icon Full hight_WIN {size}x{size}.png"
    if png_file.exists():
        images.append((size, png_file.read_bytes()))

if not images:
    raise SystemExit(f"No PNG icon files found in {icon_dir}")

header = struct.pack("<HHH", 0, 1, len(images))
offset = 6 + 16 * len(images)
entries = []
payload = []

for size, data in images:
    width = size if size < 256 else 0
    height = size if size < 256 else 0
    entries.append(struct.pack("<BBBBHHII", width, height, 0, 0, 1, 32, len(data), offset))
    payload.append(data)
    offset += len(data)

output_ico.write_bytes(header + b"".join(entries) + b"".join(payload))
PY
fi

if [[ -f "$OUTPUT_ICO" ]]; then
    echo ""
    echo "Successfully created: ${OUTPUT_ICO}"
    ls -la "$OUTPUT_ICO"
else
    echo "Error: Failed to create ${OUTPUT_ICO}"
    exit 1
fi
