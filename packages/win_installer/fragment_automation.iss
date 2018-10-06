; Copyright (c) 2007-2009, Niels Martin Hansen
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
;   * Redistributions of source code must retain the above copyright notice,
;     this list of conditions and the following disclaimer.
;   * Redistributions in binary form must reproduce the above copyright notice,
;     this list of conditions and the following disclaimer in the documentation
;     and/or other materials provided with the distribution.
;   * Neither the name of the Aegisub Group nor the names of its contributors
;     may be used to endorse or promote products derived from this software
;     without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; -----------------------------------------------------------------------------
;
; AEGISUB
;
; Website: http://www.aegisub.org/
; Contact: mailto:nielsm@indvikleren.dk
;

; This file declares all installables related to Aegisub Automation

[Files]
DestDir: {app}\automation\autoload; Source: ..\..\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: ..\..\automation\autoload\karaoke-auto-leadin.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: ..\..\automation\autoload\kara-templater.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: ..\..\automation\autoload\select-overlaps.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled
DestDir: {app}\automation\autoload; Source: ..\..\automation\autoload\strip-tags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\bundled

DestDir: {app}\automation\demos; Source: ..\..\automation\demos\future-windy-blur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos
DestDir: {app}\automation\demos; Source: ..\..\automation\demos\raytracer.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: macros\demos

DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\argcheck.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\clipboard.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\ffi.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\lfs.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\re.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\unicode.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include\aegisub; Source: ..\..\automation\include\aegisub\util.moon; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main

DestDir: {app}\automation\include; Source: ..\..\automation\include\cleantags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\clipboard.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\karaskel.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\karaskel-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\lfs.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\moonscript.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\re.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\unicode.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\utils.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
DestDir: {app}\automation\include; Source: ..\..\automation\include\utils-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main

; DepCtrl
#ifdef DEPCTRL
DestDir: {userappdata}\Aegisub\automation\include\l0; Source: vendor\DependencyControl\modules\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\autoload; Source: vendor\DependencyControl\macros\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include; Source: vendor\Yutils\src\Yutils.lua; Flags: ignoreversion; Components: macros\modules\yutils
DestDir: {userappdata}\Aegisub\automation\include; Source: vendor\ffi-experiments\luajson\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\luajson
DestDir: {userappdata}\Aegisub\automation\include\requireffi; Source: vendor\ffi-experiments\requireffi\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl

DestDir: {userappdata}\Aegisub\automation\include; Source: vendor\ffi-experiments\BadMutex\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include; Source: vendor\ffi-experiments\PreciseTimer\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl
DestDir: {userappdata}\Aegisub\automation\include; Source: vendor\ffi-experiments\DownloadManager\*; Flags: ignoreversion recursesubdirs createallsubdirs; Components: macros\modules\depctrl

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
