; This file declares all installables related to Aegisub Automation

[Files]
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\karaoke-auto-leadin.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\kara-templater.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\select-overlaps.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\strip-tags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled

DestDir: {app}\automation\demos; Source: {#SOURCE_ROOT}\automation\demos\future-windy-blur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos
DestDir: {app}\automation\demos; Source: {#SOURCE_ROOT}\automation\demos\raytracer.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos

DestDir: {app}\automation\include\aegisub\internal; Source: {#SOURCE_ROOT}\automation\include\aegisub\internal\argcheck.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub\internal; Source: {#SOURCE_ROOT}\automation\include\aegisub\internal\ffi.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main

DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\clipboard.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\lfs.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\re.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\unicode.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\util.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main

DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\cleantags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\clipboard.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\karaskel.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\karaskel-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\lfs.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\moonscript.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\re.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\unicode.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\unicode-monkeypatch.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\utils.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: {#SOURCE_ROOT}\automation\include\utils-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main

#ifdef DEPCTRL
; DepCtrl's release bundle already has the files arranged for Aegisub's automation directory layout.
;
; Yutils, luajson and the ffi-experiments libraries are no longer bundled. DependencyControl
; installs Yutils from its own feed and ships internal replacements for json, BadMutex, PreciseTimer and
; DownloadManager.
DestDir: {userappdata}\Aegisub\automation; Source: {#DEPS_DIR}\DependencyControl\automation\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
#endif

[InstallDelete]
Type: files; Name: "{userappdata}\Aegisub\l0.UpdateFeed_*.json"
Type: files; Name: "{userappdata}\Aegisub\DependencyControl.json"     
Type: files; Name: "{userappdata}\Aegisub\Nudge.json"
Type: files; Name: "{userappdata}\Aegisub\PasteAILines.json"
Type: files; Name: "{userappdata}\Aegisub\ASSWipe.json"
Type: files; Name: "{userappdata}\Aegisub\automation\include\DM\DownloadManager.dll"
Type: files; Name: "{userappdata}\Aegisub\automation\include\BM\BadMutex.dll"
Type: files; Name: "{userappdata}\Aegisub\automation\include\PT\PreciseTimer.dll"

#ifdef CLEANUP_UNBUNDLED_MODULES
; Remove the module copies earlier versions bundled, so the DependencyControl-managed ones take
; over. Lua resolves these paths before DependencyControl's own module search, so left in place
; they keep shadowing the managed copies and never migrate.
;
; Opt-in, because it also removes them from scripts that use these modules directly without
; DependencyControl and thus cannot reach the replacements.
Type: files; Name: "{userappdata}\Aegisub\automation\include\Yutils.lua"
Type: files; Name: "{userappdata}\Aegisub\automation\include\json.lua"
Type: filesandordirs; Name: "{userappdata}\Aegisub\automation\include\json"
Type: filesandordirs; Name: "{userappdata}\Aegisub\automation\include\requireffi"
Type: filesandordirs; Name: "{userappdata}\Aegisub\automation\include\BM"
Type: filesandordirs; Name: "{userappdata}\Aegisub\automation\include\PT"
Type: filesandordirs; Name: "{userappdata}\Aegisub\automation\include\DM"
#endif
