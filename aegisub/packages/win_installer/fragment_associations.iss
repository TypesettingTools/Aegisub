; Copyright (c) 2007-2011, Niels Martin Hansen
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


[Files]
DestDir: {commontemplates}; Source: template.ass; DestName: Aegisub.ass; Components: main

[Registry]
; File type registration
; Application registration for Open With dialogue
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "@{app}\aegisub32.exe,-10000"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\shell"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Applications\aegisub32.exe\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\shell\open"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "@{app}\aegisub32.exe,-10000"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Applications\aegisub32.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%1"""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".ass"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".ssa"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".srt"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".sub"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".ttxt"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".txt"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".mkv"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".mka"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".mks"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".avi"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".mp3"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".mp4"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".aac"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".m4a"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".wav"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".ogg"; ValueData: ""; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Applications\aegisub32.exe\SupportedTypes"; ValueType: string; ValueName: ".avs"; ValueData: ""; Flags: uninsdeletekey
; Class for general subtitle formats
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Subtitle.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub subtitle file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Subtitle.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Subtitle.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10101"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Subtitle.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Subtitle.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Subtitle.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Subtitle.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .ass files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.ASSA.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub Advanced SSA subtitles"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.ASSA.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.ASSA.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10102"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.ASSA.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.ASSA.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.ASSA.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.ASSA.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .ssa files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SSA.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub SubStation Alpha subtitles"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SSA.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SSA.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10103"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SSA.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SSA.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.SSA.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.SSA.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .srt files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SRT.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub SubRip text subtitles"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SRT.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SRT.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10104"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SRT.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.SRT.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.SRT.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.SRT.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .ttxt files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TTXT.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub MPEG-4 timed text"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TTXT.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TTXT.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10105"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TTXT.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TTXT.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.TTXT.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.TTXT.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .mks files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.MKS.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub Matroska subtitles"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.MKS.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.MKS.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10106"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.MKS.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.MKS.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.MKS.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.MKS.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for .txt files
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TXT.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub raw text file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TXT.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TXT.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10107"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TXT.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.TXT.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.TXT.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.TXT.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for undecideable media file types
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Media.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub media file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Media.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Media.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10108"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Media.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Media.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Media.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Media.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for audio file types
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Audio.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub audio file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Audio.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Audio.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10109"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Audio.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Audio.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Audio.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Audio.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Class for video file types
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Video.1"; ValueType: string; ValueName: ""; ValueData: "Aegisub video file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Video.1"; ValueType: dword; ValueName: "EditFlags"; ValueData: $af0; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Video.1"; ValueType: string; ValueName: "FriendlyTypeName"; ValueData: "@{app}\aegisub32.exe,-10110"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Video.1\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe,0"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\Aegisub.Video.1\shell"; ValueType: string; ValueName: ""; ValueData: "open"; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Video.1\shell\open"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; SubKey: "SOFTWARE\Classes\Aegisub.Video.1\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\aegisub32.exe"" ""%L"""; Flags: uninsdeletekey
; Default Programs registration
Root: HKLM; Subkey: "SOFTWARE\Aegisub"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities"; ValueType: none
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "@{app}\aegisub32.exe,-10001"
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: none
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: string; ValueName: ".ass"; ValueData: "Aegisub.ASSA.1"
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: string; ValueName: ".ssa"; ValueData: "Aegisub.SSA.1"
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: string; ValueName: ".srt"; ValueData: "Aegisub.SRT.1"
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: string; ValueName: ".ttxt"; ValueData: "Aegisub.TTXT.1"
Root: HKLM; Subkey: "SOFTWARE\Aegisub\Capabilities\FileAssociations"; ValueType: string; ValueName: ".mks"; ValueData: "Aegisub.MKS.1"
Root: HKLM; Subkey: "SOFTWARE\RegisteredApplications"; ValueType: string; ValueName: "Aegisub"; ValueData: "SOFTWARE\Aegisub\Capabilities"; Flags: uninsdeletevalue
; Default handler for .ass
; Only register us as owner of the type if there isn't one already. Windows XP doesn't cooperate well when a type has no owner,
; even if everything else exists. If it already has an owner, use some other UI to take over ownership if the user desires.
; Only set perceived types for the main text-based subtitle formats.
; We only have a template file for .ass, that's the primary subtitle format.
; Register us as a valid ProgID for every type we can reasonably handle, and a few we might not be able to.
Root: HKLM; SubKey: "SOFTWARE\Classes\.ass"; ValueType: string; ValueData: "Aegisub.ASSA.1"; Flags: createvalueifdoesntexist
Root: HKLM; SubKey: "SOFTWARE\Classes\.ass"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "SOFTWARE\Classes\.ass\Aegisub.ASSA.1"; ValueType: none; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.ass\Aegisub.ASSA.1\ShellNew"; ValueType: string; ValueName: "FileName"; ValueData: "{commontemplates}\Aegisub.ass"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\.ass\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.ASSA.1"; Flags: uninsdeletevalue
; Default handler for .ssa
Root: HKLM; SubKey: "SOFTWARE\Classes\.ssa"; ValueType: string; ValueData: "Aegisub.SSA.1"; Flags: createvalueifdoesntexist
Root: HKLM; SubKey: "SOFTWARE\Classes\.ssa"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "SOFTWARE\Classes\.ssa\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.SSA.1"; Flags: uninsdeletevalue
; Default handler for .srt
Root: HKLM; SubKey: "SOFTWARE\Classes\.srt"; ValueType: string; ValueData: "Aegisub.SRT.1"; Flags: createvalueifdoesntexist
Root: HKLM; SubKey: "SOFTWARE\Classes\.srt"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "SOFTWARE\Classes\.srt\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.SRT.1"; Flags: uninsdeletevalue
; Default handler for .ttxt
Root: HKLM; SubKey: "SOFTWARE\Classes\.ttxt"; ValueType: string; ValueData: "Aegisub.TTXT.1"; Flags: createvalueifdoesntexist
Root: HKLM; SubKey: "SOFTWARE\Classes\.ttxt"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "SOFTWARE\Classes\.ttxt\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.TTXT.1"; Flags: uninsdeletevalue
; Default handler for .mks
Root: HKLM; SubKey: "SOFTWARE\Classes\.mks"; ValueType: string; ValueData: "Aegisub.MKS.1"; Flags: createvalueifdoesntexist
Root: HKLM; SubKey: "SOFTWARE\Classes\.mks"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "SOFTWARE\Classes\.mks\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.MKS.1"; Flags: uninsdeletevalue
; Support opening a bunch more types
Root: HKLM; Subkey: "SOFTWARE\Classes\.sub\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Subtitle.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.txt\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.TXT.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.mkv\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Video.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.mka\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Audio.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.avi\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Video.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.mp3\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Audio.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.mp4\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Media.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.aac\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Audio.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.m4a\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Audio.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.wav\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Audio.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.ogg\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Media.1"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Classes\.avs\OpenWithProgids"; ValueType: string; ValueName: "Aegisub.Video.1"; Flags: uninsdeletevalue

