#!/usr/bin/env bash

set -euo pipefail

PREFIX="${PREFIX:-/usr/local}"
DESTDIR="${DESTDIR:-}"

echo "Installing mdp-generator..."
make install PREFIX="$PREFIX" DESTDIR="$DESTDIR"
echo "Installation complete."
