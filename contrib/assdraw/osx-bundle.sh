#!/bin/sh

set -e

BUNDLE_ROOT="$(dirname $0)/Assdraw.app/"
SRC_DIR="$(dirname $0)"

if test -d "${BUNDLE_ROOT}"; then
  rm -rf "${BUNDLE_ROOT}"
fi

mkdir -p "${BUNDLE_ROOT}/Contents/MacOS"

cp "${SRC_DIR}/src/assdraw" "${BUNDLE_ROOT}/Contents/MacOS/assdraw"

python ../../aegisub/tools/osx-fix-libs.py "${BUNDLE_ROOT}/Contents/MacOS/assdraw"

cat << 'EOF' > "${BUNDLE_ROOT}/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>assdraw</string>
	<key>CFBundleIdentifier</key>
	<string>com.aegisub.assdraw</string>
	<key>CFBundleName</key>
	<string>ASSDraw3</string>
	<key>CFBundleDisplayName</key>
	<string>ASSDraw3</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>LSHasLocalizedDisplayName</key>
	<false/>
</dict>
</plist>
EOF
