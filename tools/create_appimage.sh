#!/usr/bin/env bash
set -euo pipefail

# Simple AppImage creator for Aegisub (x86_64 or ARM64 Linux)
# Usage: tools/create_appimage.sh [build-dir]
# Requirements:
#  - meson & ninja
#  - Linux x86_64 or ARM64 host
#  - appimagetool (will be downloaded automatically if not found)

BUILD_DIR=${1:-build}
APPDIR=AppDir
APPNAME=Aegisub
APP_VERSION=${APP_VERSION:-3.5.0}
case "${APPIMAGE_ARCH:-$(uname -m)}" in
  x86_64|amd64)
    APPIMAGE_ARCH=x86_64
    APPIMAGETOOL_ARCH=x86_64
    ;;
  aarch64|arm64)
    APPIMAGE_ARCH=arm64
    APPIMAGETOOL_ARCH=aarch64
    ;;
  *)
    echo "Unsupported AppImage architecture: $(uname -m). Use x86_64 or aarch64." >&2
    exit 1
    ;;
esac
OUTFILE=${APPNAME}-v${APP_VERSION}-${APPIMAGE_ARCH}.AppImage
MESON_CROSS_FILE=${MESON_CROSS_FILE:-}

if [ "$(uname -s)" != "Linux" ]; then
  echo "This script must be run on Linux." >&2
  exit 1
fi
# Configure and build (static default_library recommended for portable bundles).
# Never reuse a cached Meson build: cached subproject state can refer to a
# different wrap revision and leave FFMS2 without its packagefile patch.
rm -rf "$BUILD_DIR"
MESON_ARGS=(-Ddefault_library=static)
if [ -n "$MESON_CROSS_FILE" ]; then
  MESON_ARGS+=(--cross-file "$MESON_CROSS_FILE")
fi
meson setup "$BUILD_DIR" "${MESON_ARGS[@]}"
meson compile -C "$BUILD_DIR"

# Install into AppDir using meson install with destdir
rm -rf "$APPDIR"
meson install -C "$BUILD_DIR" --destdir "$PWD/$APPDIR"

# Ensure AppRun exists
cat > "$APPDIR/AppRun" <<'EOF'
#!/usr/bin/env bash
HERE="$(dirname "$(readlink -f "$0")")"
exec "$HERE/usr/bin/aegisub" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Ensure desktop file exists
DESKTOP_DIR="$APPDIR/usr/share/applications"
mkdir -p "$DESKTOP_DIR"
if [ ! -f "$DESKTOP_DIR/aegisub.desktop" ]; then
  cat > "$DESKTOP_DIR/aegisub.desktop" <<'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=Aegisub
GenericName=Subtitle Editor
Comment=Create and edit subtitles for film and videos.
Exec=aegisub %f
TryExec=aegisub
Icon=aegisub
Terminal=false
Categories=AudioVideo;AudioVideoEditing;GTK;
Keywords=subtitles;subtitle;captions;captioning;video;audio;
MimeType=application/x-srt;text/plain;text/x-ass;text/x-microdvd;text/x-ssa;
StartupNotify=true
StartupWMClass=aegisub
EOF
fi

# Try to find an icon source (png/svg/icns) in repository and convert to a 256x256 PNG for AppImage
ICON_DEST="$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$ICON_DEST"
ICON_SRC=""
# Common locations (png, svg, icns)
for p in "packages/icons/aegisub.png" "packages/icons/aegisub.svg" "packages/icons/256x256/apps/aegisub.png" "packages/osx_bundle/Contents/Resources/aegisub.svg" "packages/osx_bundle/Contents/Resources/Aegisub.icns"; do
  if [ -f "$p" ]; then
    ICON_SRC="$p"
    break
  fi
done

if [ -n "$ICON_SRC" ]; then
  case "$ICON_SRC" in
    *.png)
      cp "$ICON_SRC" "$ICON_DEST/aegisub.png" || true
      ;;
    *.svg)
      # Try to convert SVG to PNG using available tools
      if command -v rsvg-convert >/dev/null 2>&1; then
        echo "Converting SVG to PNG with rsvg-convert"
        rsvg-convert -w 256 -h 256 "$ICON_SRC" -o "$ICON_DEST/aegisub.png" || true
      elif command -v inkscape >/dev/null 2>&1; then
        echo "Converting SVG to PNG with inkscape"
        inkscape "$ICON_SRC" --export-type=png --export-filename="$ICON_DEST/aegisub.png" --export-width=256 --export-height=256 || true
      elif command -v convert >/dev/null 2>&1; then
        echo "Converting SVG to PNG with ImageMagick convert"
        convert "$ICON_SRC" -resize 256x256 "$ICON_DEST/aegisub.png" || true
      else
        echo "SVG icon found but no SVG->PNG conversion tool available; copying raw SVG as fallback" >&2
        cp "$ICON_SRC" "$ICON_DEST/aegisub.svg" || true
      fi
      ;;
    *.icns)
      # Try to convert .icns to png if iconutil/convert present
      if command -v iconutil >/dev/null 2>&1 && command -v sips >/dev/null 2>&1; then
        echo "Converting .icns to PNG using iconutil/sips"
        tmpdir=$(mktemp -d)
        iconutil -c iconset "$ICON_SRC" -o "$tmpdir/aicon.iconset" >/dev/null 2>&1 || true
        # find the largest png
        if [ -d "$tmpdir/aicon.iconset" ]; then
          cp "$tmpdir/aicon.iconset"/*.png "$ICON_DEST/" || true
          # pick a 256x256 if present
          if [ -f "$ICON_DEST/icon_256x256.png" ]; then
            cp "$ICON_DEST/icon_256x256.png" "$ICON_DEST/aegisub.png" || true
          else
            # fallback: pick any png
            cp "$ICON_DEST"/*.png "$ICON_DEST/aegisub.png" || true
          fi
        fi
        rm -rf "$tmpdir" || true
      else
        echo "Icon source is .icns but no conversion tools available; skipping icon conversion." >&2
      fi
      ;;
  esac
fi

# Ensure appimagetool available
APPIMAGETOOL=${APPIMAGETOOL:-$HOME/.cache/appimagetool/appimagetool-${APPIMAGETOOL_ARCH}.AppImage}
if [ ! -x "$APPIMAGETOOL" ]; then
  echo "Downloading appimagetool..."
  mkdir -p "$(dirname "$APPIMAGETOOL")"
  TMPDL="$(mktemp)"
  curl -L --fail -o "$TMPDL" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${APPIMAGETOOL_ARCH}.AppImage"
  chmod +x "$TMPDL"
  mv "$TMPDL" "$APPIMAGETOOL"
fi

# Build AppImage
"$APPIMAGETOOL" "$APPDIR" "$OUTFILE"

echo "Built $OUTFILE"
