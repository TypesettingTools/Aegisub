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
; auto4 main
DestDir: {app}\automation\include; Source: src\automation\include\utils.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\utils-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\unicode.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\cleantags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\autoload; Source: src\automation\autoload\kara-templater.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
; auto4 samples
DestDir: {app}\automation\demos; Source: src\automation\demos\future-windy-blur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: src\automation\autoload\macro-1-edgeblur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: src\automation\autoload\macro-2-mkfullwitdh.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: src\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
; auto3
DestDir: {app}\automation\include; Source: src\automation\include\utils.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel-adv.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel-adv.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel-base.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: src\automation\include\karaskel-base.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: src\automation\auto3\line-per-syllable.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: src\automation\auto3\multi-template.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: src\automation\auto3\simple-k-replacer.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\docs; Source: src\automation\docs\automation3.txt; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly

[Components]
; Automation
Name: auto; Description: Automation 4 scripting support; Types: compact full
Name: auto/lua; Description: Lua; Types: compact full; Flags: checkablealone; Languages:
Name: auto/lua/samples; Description: Lua sample scripts; Types: full
Name: auto/auto3; Description: Automation 3 backwards compatibility; Types: full

