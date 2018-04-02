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

[Components]
Name: "main"; Description: "Main Files"; Types: full compact custom; Flags: fixed
Name: "macros"; Description: "Automation Scripts"; Types: full
Name: "macros\bundled"; Description: "Bundled macros"; Types: full
Name: "macros\demos"; Description: "Example macros/Demos"; Types: full
Name: "macros\modules"; Description: "Modules"; Types: full
Name: "macros\modules\depctrl"; Description: "DependencyControl"; Types: full
Name: "macros\modules\yutils"; Description: "YUtils"; Types: full
Name: "macros\modules\luajson"; Description: "LuaJSON"; Types: full
Name: "dictionaries"; Description: "Spellcheck Dictionaries"; Types: full
Name: "dictionaries\en_US"; Description: "English (US)"; Types: full
Name: "translations"; Description: "Aegisub Translations"; Types: full
Name: "assdraw"; Description: "ASSDraw 3"; Types: full

[Tasks]
Name: "startmenuicon"; Description: "{cm:StartMenuIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "checkforupdates"; Description: "{cm:CheckForUpdates}"; GroupDescription: "{cm:UpdatesGroup}"

[Files]
; main
DestDir: {app}; Source: ..\..\bin\aegisub{#ARCH}.exe; Flags: ignoreversion; Components: main
DestDir: {app}; Source: license.txt; Flags: ignoreversion; Components: main

[Icons]
Name: {commonprograms}\Aegisub; Filename: {app}\aegisub{#ARCH}.exe; WorkingDir: {app}; IconIndex: 0; Tasks: startmenuicon; Comment: Create and edit subtitle files

[Registry]
; Register in App Paths so the user can conveniently enter 'aegisub' in their Run box
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\aegisub{#ARCH}.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub{#ARCH}.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\aegisub.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub{#ARCH}.exe"; Flags: uninsdeletekey

[Run]
Filename: {app}\aegisub{#ARCH}.exe; Description: {cm:LaunchProgram,Aegisub}; Flags: nowait postinstall skipifsilent

[InstallDelete]
Type: files; Name: "{app}\ffms2_64.dll"
Type: files; Name: "{app}\ffms2_32.dll"
