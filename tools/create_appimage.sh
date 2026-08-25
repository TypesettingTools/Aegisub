#!/usr/bin/env bash
set -euo pipefail

# Simple AppImage creator for Aegisub (x86_64 Linux)
# Usage: tools/create_appimage.sh [build-dir]
# Requirements:
#  - meson & ninja
#  - linux x86_64 host
#  - appimagetool (will be downloaded automatically if not found)

BUILD_DIR=${1:-build}
APPDIR=AppDir
APPNAME=Aegisub
OUTFILE=${APPNAME}-x86_64.AppImage

if [ "$(uname -s)" != "Linux" ]; then
  echo "This script must be run on Linux." >&2
  exit 1
fi
if [ "$(uname -m)" != "x86_64" ]; then
  echo "Only x86_64 AppImage builds are supported by this script." >&2
  exit 1
fi

# Configure and build (static default_library recommended for portable bundles)
meson setup "$BUILD_DIR" --reconfigure -Ddefault_library=static || true
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

# Try to find an icon png in repository (common locations), else try to extract from .icns
ICON_DEST="$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$ICON_DEST"
ICON_SRC=""
# Common locations
for p in "packages/icons/aegisub.png" "packages/icons/256x256/apps/aegisub.png" "packages/osx_bundle/Contents/Resources/Aegisub.icns"; do
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
APPIMAGETOOL=${APPIMAGETOOL:-$HOME/.cache/appimagetool/appimagetool-x86_64.AppImage}
if [ ! -x "$APPIMAGETOOL" ]; then
  echo "Downloading appimagetool..."
  mkdir -p "$(dirname "$APPIMAGETOOL")"
  TMPDL="/tmp/appimagetool.AppImage"
  curl -L -o "$TMPDL" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
  chmod +x "$TMPDL"
  mv "$TMPDL" "$APPIMAGETOOL"
fi

# Build AppImage
"$APPIMAGETOOL" "$APPDIR" "$OUTFILE"

echo "Built $OUTFILE"
