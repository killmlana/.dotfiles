#!/bin/bash
set -e

EWW_DIR="$HOME/.config/eww"

if [ -d "$EWW_DIR" ] && [ -f "$EWW_DIR/eww.yuck" ]; then
    echo "Existing eww config found at $EWW_DIR"
    echo "Back it up before continuing: mv $EWW_DIR $EWW_DIR.bak"
    exit 1
fi

echo "Building binaries..."
make all

echo "Installing to $EWW_DIR..."
make install

echo ""
echo "Done! Run 'eww daemon' and 'eww open bartest' to get started."
