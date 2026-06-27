#!/usr/bin/env bash

set -euo pipefail

PREFIX="${PREFIX:-/usr}"
DESTDIR="${DESTDIR:-}"

echo "Installing mdp-generator..."
make install PREFIX="$PREFIX" DESTDIR="$DESTDIR"

if [ -z "$DESTDIR" ]; then
  if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${PREFIX}/share/applications" || true
  fi

  if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor" || true
  fi
fi

echo "Installation complete."
