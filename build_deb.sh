#!/usr/bin/env bash

set -euo pipefail

PACKAGE_NAME="mdp-generator"
PACKAGE_VERSION="${PACKAGE_VERSION:-0.1.0}"
ARCH="$(dpkg --print-architecture)"
ROOT_DIR="$(pwd)"
BUILD_DIR="${ROOT_DIR}/dist/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}"
DEBIAN_DIR="${BUILD_DIR}/DEBIAN"
OUTPUT_DEB="${ROOT_DIR}/dist/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb"

rm -rf "${BUILD_DIR}"
mkdir -p "${DEBIAN_DIR}"

make install DESTDIR="${BUILD_DIR}" PREFIX=/usr

cat > "${DEBIAN_DIR}/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: Guillaume Boileau <guillaume.boileau@oca.eu>
Depends: libgtk-3-0, libasound2, libssl3
Description: Microphone-based password generator for Ubuntu
 Generates passwords from microphone noise with a GTK interface,
 SHA-256 hashing, and modulo-bias-free character selection.
EOF

cat > "${DEBIAN_DIR}/postinst" <<'EOF'
#!/usr/bin/env bash
set -e

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database /usr/share/applications || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
fi
EOF

cat > "${DEBIAN_DIR}/postrm" <<'EOF'
#!/usr/bin/env bash
set -e

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database /usr/share/applications || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
fi
EOF

chmod 0755 "${DEBIAN_DIR}/postinst" "${DEBIAN_DIR}/postrm"

mkdir -p "${ROOT_DIR}/dist"
dpkg-deb --build "${BUILD_DIR}" "${OUTPUT_DEB}"

echo "Created ${OUTPUT_DEB}"
