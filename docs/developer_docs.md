# Developer Documentation

This file collects some miscellaneous information relevant to Aegisub development.

## Releasing a new Version
Follow the following steps to release a new Aegisub version:

- Bump Aegisub's version number in the following places:
    - `meson.build`
    - `packages/win_installer/portable-comment.txt` (two occurrences)
    - `po/make_pot.sh` (two occurrences)
- Make a release candidate build (including a string/feature freeze) and ping translators with a time interval to update the translations
- For larger updates: Cut a new version of Aegisub's manual on the website and update the link in `src/help_button.cpp`.
- Add the new version with its release date to `aegisub.metainfo.xml.in.in`
- Create and push a new version tag
- Upload the CI builds to a GitHub release
- Link the new release on Aegisub's website
- Add the release to the update checking server
- If needed, create a support branch for backports, keeping the master branch free for larger changes

### macOS release signing

Tagged and manually dispatched builds upload an ad-hoc-signed
`-signing-input.zip`. This is an input to the release process, not a
distributable release artifact. Publish only the Developer ID-signed and
notarized DMG produced below.

Create a `notarytool` profile once. Add `--keychain PATH` if the profile
should be stored in a keychain other than the login keychain:

```bash
export AEGISUB_NOTARY_PROFILE=aegisub-release
xcrun notarytool store-credentials "${AEGISUB_NOTARY_PROFILE}"
```

Check out the exact commit which produced the CI artifact. Downloads from the
GitHub web UI are wrapper ZIPs containing the uploaded `-signing-input.zip`.
Extract that wrapper first:

```bash
mkdir -p signing-artifact
ditto -x -k /path/to/github-artifact-download.zip signing-artifact
```

Alternatively, GitHub CLI performs that outer extraction while downloading:

```bash
gh run download RUN_ID \
  --name 'macOS arm64 Release - local signing input' \
  --dir signing-artifact
```

Then extract the signing input into the staging directory. The resulting path
must be `build/Aegisub.app`:

```bash
mkdir -p build
ditto -x -k signing-artifact/Aegisub-*-signing-input.zip build
```

This release-manager step does not rebuild or resolve Aegisub's dependencies.
It requires the matching source checkout, Xcode command-line tools, the
Developer ID certificate, the stored notary profile, and the extracted app;
Meson, CMake, Ninja, Homebrew, and network access to the wrap sources are not
required. Export the release identity and notary profile, then run the three
scripts in order from the repository root:

```bash
export AEGISUB_BUNDLE_SIGNATURE='Developer ID Application: Example (TEAMID)'
export AEGISUB_NOTARY_PROFILE=aegisub-release

# Optional when using non-default keychains:
export AEGISUB_SIGNING_KEYCHAIN=/path/to/signing.keychain-db
export AEGISUB_NOTARY_KEYCHAIN=/path/to/notary.keychain-db

tools/osx-sign.sh "$PWD" "$PWD/build/Aegisub.app"
tools/osx-dmg.sh "$PWD" "$PWD/build"
tools/osx-notarize.sh "$PWD" "$PWD/build"
```

The final notarization submission requires internet access to Apple's notary
service.

The complete script interfaces are:

```text
tools/osx-sign.sh SOURCE_DIR AEGISUB_APP
tools/osx-dmg.sh SOURCE_DIR BUILD_DIR [VERSION_OVERRIDE]
tools/osx-notarize.sh SOURCE_DIR BUILD_DIR [VERSION_OVERRIDE]
```

An existing, fully configured Meson build offers equivalent `osx-sign`,
`osx-build-dmg`, and `osx-notarize` targets, but setting up a fresh build only
to obtain these wrappers unnecessarily resolves the full dependency tree.

`AEGISUB_BUNDLE_SIGNATURE` is mandatory. Set it to `-` only for an explicitly
ad-hoc CI or development build; such an image cannot be notarized.
`AEGISUB_BUNDLE_ENTITLEMENTS` optionally replaces the default entitlements
file, and `AEGISUB_NOTARY_TIMEOUT` optionally replaces the default `30m`
submission timeout.

Hardened-runtime library validation deliberately remains enabled. Bundled and
third-party native Automation modules must therefore be Apple-signed or signed
with the same Team ID as Aegisub. Rebuild and sign controlled modules rather
than disabling library validation for the whole application.

The DMG is the outermost distributed container, so it is the item submitted to
the notary service and stapled. This follows Apple's nested-container guidance;
the app and every nested Mach-O file are still Developer ID-signed before the
DMG is created.

## Intel macOS cross builds

The Intel Release CI lane uses an Apple Silicon runner with
`--cross-file tools/macos-release.ini --cross-file tools/macos-x86_64.ini`.
Compilation uses native Apple Silicon tools targeting x86_64; Rosetta runs
the resulting Intel test executables.
This checks the Intel code path, but does not replace testing a release on
real Intel hardware.

The Intel lane builds its target dependencies from source using the shared
macOS release dependency options in `.github/workflows/ci.yml`. Both Release
lanes use `tools/macos-release.ini` to restrict pkg-config and CMake dependency
discovery away from Homebrew; arm64 loads it with `--native-file`. Homebrew is
still used for build tools. The native arm64 Debug lane still uses
Homebrew libraries and is tested but not packaged. Intel coverage comes from
the Release lane, including tests under Rosetta.

For a local cross build, use the same dependency options and cross files as CI.
After Meson setup, run `tools/macos-build-fftw.sh BUILD_DIR`, then reconfigure
with `-Dpkg_config_path="$PWD/BUILD_DIR/fftw-prefix/lib/pkgconfig"`
and `-Dfftw3=enabled`. The bootstrap reads the target architecture from Meson
and only passes Autoconf's `--host` option when it differs from the build CPU.
Automation tests also need Intel builds of Busted's native Lua modules; the
workflow shows the LuaRocks compiler overrides. Use a separate LuaRocks tree
if native arm64 tests need to run on the same machine.

## Running Doxygen

You can run Doxygen with the following command:

```bash
doxygen docs/doxygen.cfg
```

This will generate API documentation in `docs/generated/api/html/`.

## Updating Moonscript

From within the Moonscript repository, run `bin/moon bin/splat.moon -l moonscript moonscript/ > bin/moonscript.lua`.
Open the newly created `bin/moonscript.lua`, and within it make the following changes:

1. Prepend the final line of the file, `package.preload["moonscript"]()`, with a `return`, producing `return package.preload["moonscript"]()`.
2. Within the function at `package.preload['moonscript.base']`, remove references to `moon_loader`, `insert_loader`, and `remove_loader`. This means removing their declarations, definitions, and entries in the returned table.
3. Within the function at `package.preload['moonscript']`, remove the line `_with_0.insert_loader()`.

The file is now ready for use, to be placed in `automation/include` within the Aegisub repo.
