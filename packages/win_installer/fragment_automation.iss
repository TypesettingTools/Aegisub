; This file declares all installables related to Aegisub Automation

[Files]
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\karaoke-auto-leadin.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\kara-templater.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\select-overlaps.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: {#SOURCE_ROOT}\automation\autoload\strip-tags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled

DestDir: {app}\automation\demos; Source: {#SOURCE_ROOT}\automation\demos\future-windy-blur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos
DestDir: {app}\automation\demos; Source: {#SOURCE_ROOT}\automation\demos\raytracer.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos

DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\argcheck.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\clipboard.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: {#SOURCE_ROOT}\automation\include\aegisub\ffi.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
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
; DepCtrl
DestDir: {userappdata}\Aegisub\automation\include\l0; Source: {#DEPS_DIR}\DependencyControl\modules\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\autoload; Source: {#DEPS_DIR}\DependencyControl\macros\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include; Source: {#DEPS_DIR}\Yutils\src\Yutils.lua; Flags: ignoreversion; Components: macros\modules\yutils
DestDir: {userappdata}\Aegisub\automation\include; Source: {#DEPS_DIR}\luajson\lua\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\luajson

DestDir: {userappdata}\Aegisub\automation\include\requireffi; Source: {#DEPS_DIR}\ffi-experiments\build\requireffi\requireffi.lua; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\BM\BadMutex; Source: {#DEPS_DIR}\ffi-experiments\build\bad-mutex\BadMutex.dll; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\BM; Source: {#DEPS_DIR}\ffi-experiments\build\bad-mutex\BadMutex.lua; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\PT\PreciseTimer; Source: {#DEPS_DIR}\ffi-experiments\build\precise-timer\PreciseTimer.dll; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\PT; Source: {#DEPS_DIR}\ffi-experiments\build\precise-timer\PreciseTimer.lua; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\DM\DownloadManager; Source: {#DEPS_DIR}\ffi-experiments\build\download-manager\DownloadManager.dll; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include\DM; Source: {#DEPS_DIR}\ffi-experiments\build\download-manager\DownloadManager.lua; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl

[Dirs]
Name: {userappdata}\Aegisub\automation\test\DepUnit\automation; Components: macros\modules\depctrl;
Name: {userappdata}\Aegisub\automation\test\DepUnit\modules; Components: macros\modules\depctrl;
Name: {userappdata}\Aegisub\automation\schema\DepSqlite\automation; Components: macros\modules\depctrl;
Name: {userappdata}\Aegisub\automation\schema\DepSqlite\modules; Components: macros\modules\depctrl;
Name: {userappdata}\Aegisub\automation\lifecycle\DepLifecycle\automation; Components: macros\modules\depctrl;
Name: {userappdata}\Aegisub\automation\lifecycle\DepLifecycle\modules; Components: macros\modules\depctrl;
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
