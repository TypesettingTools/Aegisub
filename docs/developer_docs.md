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

