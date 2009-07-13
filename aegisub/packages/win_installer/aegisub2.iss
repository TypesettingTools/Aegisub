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


[Setup]
AppID={{24BC8B57-716C-444F-B46B-A3349B9164C5}
AppName=Aegisub
AppVerName=Aegisub 2.1.7
AppVersion=2.1.7
AppPublisher=Aegisub Team
AppPublisherURL=http://www.aegisub.org/
AppSupportURL=http://forum.aegisub.org/
AppCopyright=© 2005-2009 The Aegisub Team
DefaultDirName={pf}\Aegisub
DefaultGroupName=Aegisub
AllowNoIcons=true
OutputDir=output
OutputBaseFilename=Aegisub-2.1.7-setup
Compression=lzma/ultra64
SolidCompression=true
MinVersion=0,5.0
ShowLanguageDialog=no
LanguageDetectionMethod=none
PrivilegesRequired=poweruser
DisableProgramGroupPage=yes
UsePreviousGroup=yes
UsePreviousSetupType=no
UsePreviousAppDir=yes
UsePreviousTasks=no
UninstallDisplayIcon={app}\aegisub32.exe
; Default to a large welcome bitmap, suitable for large fonts
; The normal fonts version is selected by code below
WizardImageFile=welcome-large.bmp
WizardSmallImageFile=aegisub-large.bmp

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; small bitmaps
DestDir: {tmp}; Flags: dontcopy; Source: welcome.bmp
DestDir: {tmp}; Flags: dontcopy; Source: aegisub.bmp
; uninstall data
DestDir: {tmp}; Flags: dontcopy; Source: legacy_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_shortcutlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_shortcutlist.txt
; redist
DestDir: {tmp}; Source: src\vcredist_x86.exe; Flags: nocompression deleteafterinstall; Check: RuntimesRequired
; main
DestDir: {app}; Source: src\aegisub32.exe; Flags: ignoreversion; Components: main
DestDir: {app}; Source: src\aegisub32.pdb; Flags: ignoreversion; Components: main/pdb
DestDir: {app}; Source: license.txt; Flags: ignoreversion; Tasks: ; Languages: ; Components: main
DestDir: {app}; Source: src\libauto3_32.dll; Flags: ignoreversion; Components: main
DestDir: {app}; Source: src\libauto3_32.pdb; Flags: ignoreversion; Components: main/pdb
; avisynth
DestDir: {app}; Source: src\devil.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: src\avisynth.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: src\DirectShowSource.dll; Flags: ignoreversion; Components: codec
; ffmpegsource
DestDir: {app}; Source: src\ffms2.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: src\ffms2.pdb; Flags: ignoreversion; Components: codec and main/pdb
; vsfilter
DestDir: {app}\csri; Source: src\csri\VSFilter.dll; Flags: ignoreversion; Components: codec/vsfilter
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
; dictionaries
Source: src\dictionaries\en_GB.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion solidbreak; Components: dic/en_GB
Source: src\dictionaries\en_GB.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/en_GB
Source: src\dictionaries\en_US.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/en_US
Source: src\dictionaries\en_US.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/en_US
Source: src\dictionaries\de_AT.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion solidbreak; Components: dic/de_AT
Source: src\dictionaries\de_DE.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/de_DE
Source: src\dictionaries\de_DE.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/de_DE
Source: src\dictionaries\es_ES.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/es_ES
Source: src\dictionaries\es_ES.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/es_ES
Source: src\dictionaries\fr_FR.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/fr_FR
Source: src\dictionaries\fr_FR.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/fr_FR
Source: src\dictionaries\it_IT.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/it_IT
Source: src\dictionaries\it_IT.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/it_IT
Source: src\dictionaries\nl_NL.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/nl_NL
Source: src\dictionaries\nl_NL.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/nl_NL
Source: src\dictionaries\pl_PL.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pl_PL
Source: src\dictionaries\pl_PL.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pl_PL
Source: src\dictionaries\pt_BR.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pt_BR
Source: src\dictionaries\pt_BR.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pt_BR
Source: src\dictionaries\pt_PT.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pt_PT
Source: src\dictionaries\pt_PT.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/pt_PT
Source: src\dictionaries\ru_RU.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/ru_RU
Source: src\dictionaries\ru_RU.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/ru_RU
Source: src\dictionaries\sk_SK.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sk_SK
Source: src\dictionaries\sk_SK.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sk_SK
Source: src\dictionaries\sl_SI.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sl_SI
Source: src\dictionaries\sl_SI.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sl_SI
Source: src\dictionaries\sv_SE.aff; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sv_SE
Source: src\dictionaries\sv_SE.dic; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: dic/sv_SE
; thesauri
Source: src\dictionaries\th_en_US.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion solidbreak; Components: th/en_US
Source: src\dictionaries\th_en_US.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/en_US
Source: src\dictionaries\th_de_DE.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion solidbreak; Components: th/de_DE
Source: src\dictionaries\th_de_DE.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/de_DE
Source: src\dictionaries\th_es_ES.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/es_ES
Source: src\dictionaries\th_es_ES.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/es_ES
Source: src\dictionaries\th_fr_FR.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/fr_FR
Source: src\dictionaries\th_fr_FR.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/fr_FR
Source: src\dictionaries\th_it_IT.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/it_IT
Source: src\dictionaries\th_it_IT.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/it_IT
Source: src\dictionaries\th_ru_RU.dat; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/ru_RU
Source: src\dictionaries\th_ru_RU.idx; DestDir: {app}\dictionaries; Flags: skipifsourcedoesntexist ignoreversion; Components: th/ru_RU
; localization (commented out ones are out of date; some don't have wxstd.mo)
Source: ..\..\po\ca.mo;          DestDir: {app}\locale\ca;    DestName: aegisub.mo; Flags: ignoreversion solidbreak; Components: i18n/ca
Source: ..\..\po\wxstd-ca.mo;    DestDir: {app}\locale\ca;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/ca
Source: ..\..\po\cs.mo;          DestDir: {app}\locale\cs;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/cs
;Source: ..\..\po\wxstd-cs.mo;    DestDir: {app}\locale\cs;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/cs
;Source: ..\..\po\da.mo;          DestDir: {app}\locale\da;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/da
;Source: ..\..\po\wxstd-da.mo;    DestDir: {app}\locale\da;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/da
Source: ..\..\po\de.mo;          DestDir: {app}\locale\de;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/de
Source: ..\..\po\wxstd-de.mo;    DestDir: {app}\locale\de;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/de
Source: ..\..\po\es.mo;          DestDir: {app}\locale\es;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/es
Source: ..\..\po\wxstd-es.mo;    DestDir: {app}\locale\es;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/es
Source: ..\..\po\fi.mo;          DestDir: {app}\locale\fi;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/fi
Source: ..\..\po\wxstd-fi.mo;    DestDir: {app}\locale\fi;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/fi
Source: ..\..\po\fr_FR.mo;       DestDir: {app}\locale\fr_FR; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/fr_FR
;Source: ..\..\po\wxstd-fr_FR.mo; DestDir: {app}\locale\fr_FR; DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/fr_FR
Source: ..\..\po\hu.mo;          DestDir: {app}\locale\hu;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/hu
Source: ..\..\po\wxstd-hu.mo;    DestDir: {app}\locale\hu;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/hu
;Source: ..\..\po\it.mo;          DestDir: {app}\locale\it;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/it
;Source: ..\..\po\wxstd-it.mo;    DestDir: {app}\locale\it;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/it
Source: ..\..\po\ja.mo;          DestDir: {app}\locale\ja;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/ja
Source: ..\..\po\wxstd-ja.mo;    DestDir: {app}\locale\ja;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/ja
;Source: ..\..\po\ko.mo;          DestDir: {app}\locale\ko;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/ko
;Source: ..\..\po\wxstd-ko.mo;    DestDir: {app}\locale\ko;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/ko
Source: ..\..\po\pt_BR.mo;       DestDir: {app}\locale\pt_BR; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/pt_BR
Source: ..\..\po\wxstd-pt_BR.mo; DestDir: {app}\locale\pt_BR; DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/pt_BR
Source: ..\..\po\ru.mo;          DestDir: {app}\locale\ru;    DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/ru
Source: ..\..\po\wxstd-ru.mo;    DestDir: {app}\locale\ru;    DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/ru
;Source: ..\..\po\zh_TW.mo;       DestDir: {app}\locale\zh_TW; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/zh_TW
;Source: ..\..\po\wxstd-zh_TW.mo; DestDir: {app}\locale\zh_TW; DestName: wxstd.mo;   Flags: ignoreversion; Components: i18n/zh_TW
; documentation
Source: src\docs\*; DestDir: {app}\docs; Flags: ignoreversion recursesubdirs solidbreak; Components: docs; Excludes: *svn
; ASSDraw3
Source: src\ASSDraw3.exe; DestDir: {app}; Flags: ignoreversion nocompression solidbreak; Components: assdraw
Source: src\ASSDraw3.chm; DestDir: {app}; Flags: ignoreversion; Components: assdraw and docs

[Icons]
Name: {commonprograms}\Aegisub; Filename: {app}\aegisub32.exe; WorkingDir: {app}; IconIndex: 0; Components: main/icons; Comment: Create and edit subtitle files
Name: {commonprograms}\ASSDraw3; Filename: {app}\ASSDraw3.exe; WorkingDir: {app}; IconIndex: 0; Components: main/icons and assdraw; Flags: createonlyiffileexists; Comment: Create vector drawings for ASS-format subtitles
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Aegisub; Filename: {app}\aegisub32.exe; WorkingDir: {app}; IconIndex: 0; Components: main/qcklnch; Comment: Create and edit subtitle files

[Registry]
; Register in App Paths so the user can conveniently enter 'aegisub' in their Run box
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\aegisub32.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\aegisub.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\aegisub32.exe"; Flags: uninsdeletekey

[Run]
Filename: {app}\aegisub32.exe; Description: {cm:LaunchProgram,Aegisub}; Flags: nowait postinstall skipifsilent
Filename: {tmp}\vcredist_x86.exe; StatusMsg: Installing runtime libraries...; Check: RuntimesRequired; Components: main/runtime; Parameters: "/q"

[Components]
; Actual program
Name: main; Description: Aegisub; Types: compact full custom; Languages: ; Flags: fixed
Name: main/runtime; Description: Runtime libraries; Check: RuntimesRequired; Flags: fixed; Types: custom compact full; ExtraDiskSpaceRequired: 4630528
Name: main/pdb; Description: Debug database (helps diagnose crashes); Types: full
Name: main/icons; Description: Start menu icon; Types: custom compact full
Name: main/qcklnch; Description: Quick launch icon; Types: custom compact full
Name: codec; Description: Media formats support; Flags: fixed; Types: custom compact full
Name: codec/vsfilter; Description: VSFilter 2.39 MPC-HC; Types: compact full custom; Flags: fixed
; Automation
Name: auto; Description: Automation 4 scripting support; Types: compact full
Name: auto/lua; Description: Lua; Types: compact full; Flags: checkablealone; Languages: 
Name: auto/lua/samples; Description: Lua sample scripts; Types: full
Name: auto/auto3; Description: Automation 3 backwards compatibility; Types: full
; Docs
Name: docs; Description: User manual; Types: custom compact full
; Translations (commented out ones are out of date)
Name: i18n;       Description: Languages; Types: full custom compact; Flags: fixed
Name: i18n/en;    Description: English (built in); Flags: fixed; Types: compact full custom
Name: i18n/pt_BR; Description: Brazilian Portuguese; Types: full
Name: i18n/ca;    Description: Catalan; Types: full
;Name: i18n/zh_TW; Description: Chinese (Traditional); Types: full
Name: i18n/cs;    Description: Czech; Types: full
;Name: i18n/da;    Description: Danish; Types: full
Name: i18n/fi;    Description: Finnish; Types: full
Name: i18n/fr_FR; Description: French; Types: full
Name: i18n/de;    Description: German; Types: full
Name: i18n/hu;    Description: Hungarian; Types: full
;Name: i18n/it;    Description: Italian; Types: full
Name: i18n/ja;    Description: Japanese; Types: full
;Name: i18n/ko;    Description: Korean; Types: full
Name: i18n/ru;    Description: Russian; Types: full
Name: i18n/es;    Description: Spanish; Types: full
; Languages support
Name: dic;       Description: Spell checker; Types: full
Name: dic/en_GB; Description: British English dictionary; Types: full
Name: dic/en_US; Description: American English dictionary; Types: full
Name: dic/nl_NL; Description: Dutch dictionary; Types: full
Name: dic/fr_FR; Description: French dictionary; Types: full
Name: dic/de_DE; Description: German dictionary; Types: full
Name: dic/de_AT; Description: Austrian German dictionary; Types: full
Name: dic/it_IT; Description: Italian dictionary; Types: full
Name: dic/pl_PL; Description: Polish dictionary; Types: full
Name: dic/pt_PT; Description: Portuguese dictionary; Types: full
Name: dic/pt_BR; Description: Brazilian Portuguese dictionary; Types: full
Name: dic/sk_SK; Description: Slovak dictionary; Types: full
Name: dic/sl_SI; Description: Slovenian dictionary; Types: full
Name: dic/ru_RU; Description: Russian dictionary; Types: full
Name: dic/es_ES; Description: Spanish dictionary; Types: full
Name: dic/sv_SE; Description: Swedish dictionary; Types: full
Name: th;       Description: Thesaurus; Types: full
Name: th/en_US; Description: American English thesaurus; Types: full
Name: th/fr_FR; Description: French thesaurus; Types: full
Name: th/de_DE; Description: German thesaurus; Types: full
Name: th/it_IT; Description: Italian thesaurus; Types: full
Name: th/ru_RU; Description: Russian thesaurus; Types: full
Name: th/es_ES; Description: Spanish thesaurus; Types: full
; AssDraw
Name: assdraw; Description: ai-chan's ASSDraw3 for ASS vector drawing; Types: full

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel2=This will install Aegisub 2.1.7 on your computer.%n%nAegisub is covered by the GNU General Public License version 2. This means you may use the application for any purpose without charge, but that no warranties of any kind are given either.%n%nSee the Aegisub website for information on obtaining the source code.


[Code]

var
  HasLegacyVersion: Boolean;
  LegacyStartMenuFolder: string;
  LegacyInstallFolder: string;
  LegacyVersionNumber: string;


function OldStartMenuFolder(Param: string): string;
begin
  if HasLegacyVersion then
    Result := LegacyStartMenuFolder
  else
    Result := ExpandConstant('{group}');
end;
function OldInstallFolder(Param: string): string;
begin
  if HasLegacyVersion then
    Result := LegacyInstallFolder
  else
    Result := ExpandConstant('{app}');
end;


function RuntimesRequired: Boolean;
var
  DisplayVersion: string;
begin
  // Check for uninstall entry for runtimes, don't bother installing if it can be uninstalled now
  // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9A25302D-30C0-39D9-BD6F-21E6EC160475}
  // Check: DisplayVersion = "9.0.30729"
  DisplayVersion := '';
  Result := RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9A25302D-30C0-39D9-BD6F-21E6EC160475}',
      'DisplayVersion', DisplayVersion);
  Result := Result and (DisplayVersion = '9.0.30729');
  
  Result := not Result;
end;


function SHAutoComplete(hWnd: Integer; dwFlags: DWORD): Integer;
external 'SHAutoComplete@shlwapi.dll stdcall delayload';
const
	SHACF_FILESYSTEM = $1;
	SHACF_FILESYS_ONLY = $10;
	SHACF_FILESYS_DIRS = $20;

procedure InitializeWizard;
var
  SmallBitmap: TFileStream;
begin
  // Thanks to ender for the following snippets
  
  // Fix bitmaps for small/large fonts
	if WizardForm.WizardBitmapImage.Height < 386 then //use smaller image when not using Large Fonts
	begin
		try
			ExtractTemporaryFile('welcome.bmp');
			SmallBitmap := TFileStream.Create(ExpandConstant('{tmp}\welcome.bmp'),fmOpenRead);
			WizardForm.WizardBitmapImage.Bitmap.LoadFromStream(SmallBitmap);
			WizardForm.WizardBitmapImage2.Bitmap := WizardForm.WizardBitmapImage.Bitmap;
			SmallBitmap.Free;
			
			ExtractTemporaryFile('aegisub.bmp');
			SmallBitmap := TFileStream.Create(ExpandConstant('{tmp}\aegisub.bmp'),fmOpenRead);
			WizardForm.WizardSmallBitmapImage.Bitmap.LoadFromStream(SmallBitmap);
		except
			Log('Error loading bitmaps: ' + GetExceptionMessage);
		finally
			SmallBitmap.Free;
		end;
	end;
	
	// Endow install dir edit box with autocomplete
	try
    SHAutoComplete(WizardForm.DirEdit.Handle, SHACF_FILESYSTEM);
  except
    Log('Could not add autocomplete to dir edit box');
  end;
end;


function BoolToStr(x: Boolean): string;
begin
  if x then Result := 'Yes' else Result := 'No';
end;


function InitializeSetup: Boolean;
begin
  HasLegacyVersion := RegValueExists(HKLM, 'SOFTWARE\Aegisub\info', 'InstVer');
  Log(Format('Legacy version found: %s', [BoolToStr(HasLegacyVersion)]));

  LegacyStartMenuFolder := 'Aegisub';
  if RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'StartMenuDir', LegacyStartMenuFolder) then
    LegacyStartMenuFolder := ExpandConstant('{userprograms}\') + LegacyStartMenuFolder;
  Log(Format('Legacy version Start menu folder: %s', [LegacyStartMenuFolder]));

  LegacyInstallFolder := ExpandConstant('{pf}\Aegisub');
  RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstallDir', LegacyInstallFolder);
  Log(Format('Legacy version install folder: %s', [LegacyInstallFolder]));
  
  LegacyVersionNumber := '1.x';
  RegQueryStringValue(HKLM, 'SOFTWARE\Aegisub\info', 'InstVer', LegacyVersionNumber);

  Result := True;
  if HasLegacyVersion then
    Result := SuppressibleMsgBox(Format('A previous installation of Aegisub %s has been detected and will be removed along with its configuration.'#13#10#13#10'Continue installing and remove old version?', [LegacyVersionNumber]), mbConfirmation, MB_YESNO, IDYES) = IDYES;
end;


procedure MigrateStyleCatalogs;
var
  OldCatalogDir: string;
  NewCatalogDir: string;
  search: TFindRec;
begin
  // Upgrade an 1.x style-catalog by moving it to {appdata}
  OldCatalogDir := OldInstallFolder('') + '\Catalog\';
  Log('-- Migrate style catalogs --');
  if DirExists(OldCatalogDir) then
  begin
    NewCatalogDir := ExpandConstant('{userappdata}\Aegisub\catalog\');
    ForceDirectories(NewCatalogDir);
    Log('Old style catalog dir: ' + OldCatalogDir);
    Log('New catalog dir: ' + NewCatalogDir);
    if FindFirst(OldCatalogDir + '*', search) then
    try
      repeat
        Log('Found style catalog: ' + OldCatalogDir + search.Name);
        if FileCopy(OldCatalogDir+search.Name, NewCatalogDir+search.Name, True) then
        begin
          Log('Copied catalog to: ' + NewCatalogDir+search.Name);
          DeleteFile(OldCatalogDir+search.Name);
        end;
      until not FindNext(search);
    finally
      FindClose(search);
      Log('Done migrating styles');
    end;
    RemoveDir(OldCatalogDir);
  end
  else
    Log('No existing style catalog collection found');
end;


procedure UninstallLegacyVersion;
var
  page: TOutputProgressWizardPage;
  file_list: TStringList;
  dir_list: TStringList;
  shortcut_list: TStringList;
  locale_list: TStringList;
  itemsdone, totalitems, i: Integer;
  curname: string;
begin
  // Uninstall Aegisub 1.x
  Log('-- Uninstall legacy version --');
  page := CreateOutputProgressPage('Uninstalling old version', 'Your old installation of Aegisub is being removed');
  try
    page.SetText('Preparing list of files', '');
    page.Show;

    Log('Load file lists');
    ExtractTemporaryFile('legacy_shortcutlist.txt');
    ExtractTemporaryFile('legacy_filelist.txt');
    ExtractTemporaryFile('legacy_locales.txt');
    ExtractTemporaryFile('legacy_dirlist.txt');
    shortcut_list := TStringList.Create;
    shortcut_list.LoadFromFile(ExpandConstant('{tmp}\legacy_shortcutlist.txt'));
    file_list := TStringList.Create;
    file_list.LoadFromFile(ExpandConstant('{tmp}\legacy_filelist.txt'));
    locale_list := TStringList.Create;
    locale_list.LoadFromFile(ExpandConstant('{tmp}\legacy_locales.txt'));
    dir_list := TStringList.Create;
    dir_list.LoadFromFile(ExpandConstant('{tmp}\legacy_dirlist.txt'));
    itemsdone := 0;
    totalitems := file_list.Count + dir_list.Count + shortcut_list.Count + locale_list.Count + 3;
    // Two extra for the registry keys and one for Start menu folder

    for i := 0 to shortcut_list.Count-1 do
    begin
      curname := LegacyStartMenuFolder + '\' + shortcut_list.Strings[i];
      page.SetText('Removing shortcuts', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove shortcut: ' + curname);
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;
    page.SetText('Removing Start menu folder', LegacyStartMenuFolder);
    page.SetProgress(itemsdone, totalitems);
    Log('Remove directory: ' + LegacyStartMenuFolder);
    if not RemoveDir(LegacyStartMenuFolder) then Log('* Directory deletion failed');
    itemsdone := itemsdone + 1;

    for i := 0 to file_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + file_list.Strings[i];
      page.SetText('Removing files', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove file: ' + curname);
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to locale_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + locale_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory recursively: ' + curname);
      if not DelTree(curname, True, True, True) then Log('* Tree deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to dir_list.Count-1 do
    begin
      curname := LegacyInstallFolder + '\' + dir_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory: ' + curname);
      if not RemoveDir(curname) then Log('* Directory deletion failed');
      itemsdone := itemsdone + 1;
    end;

    page.SetText('Removing registry entries', 'Installation data');
    page.SetProgress(itemsdone, totalitems);
    curname := 'SOFTWARE\Aegisub';
    Log('Remove reg key: HKLM\' + curname);
    if not RegDeleteKeyIncludingSubkeys(HKLM, curname) then Log('* Failed recursively deleting key');

    page.SetText('Removing registry entries', 'Uninstaller entry');
    page.SetProgress(itemsdone+1, totalitems);
    curname := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aegisub ' + LegacyVersionNumber;
    Log('Remove reg key: HKLM\' + curname);
    if not RegDeleteKeyIncludingSubkeys(HKLM, curname) then Log('* Failed recursively deleting key');

    page.SetText('Uninstallation complete', '');
    page.SetProgress(totalitems, totalitems);

  finally
    shortcut_list.Free;
    file_list.Free;
    locale_list.Free;
    dir_list.Free;
    page.Hide;
  end;
end;


function SetFileAttributes(lpFileName: PChar; dwFileAttributes: Longint): Boolean;
external 'SetFileAttributesA@kernel32.dll stdcall';

procedure CleanUpOldVersion;
var
  StartMenuFolder, InstallFolder: string;
  page: TOutputProgressWizardPage;
  file_list: TStringList;
  dir_list: TStringList;
  shortcut_list: TStringList;
  locale_list: TStringList;
  itemsdone, totalitems, i: Integer;
  curname: string;
begin
  // Clean up from previous Aegisub 2.x installs
  Log('-- Clean up old versions --');
  page := CreateOutputProgressPage('Cleaning old versions', 'Cleaning up from older versions of Aegisub 2');
  try
    page.SetText('Preparing list of files', '');
    page.Show;

    Log('Load file lists');
    ExtractTemporaryFile('old_shortcutlist.txt');
    ExtractTemporaryFile('old_filelist.txt');
    ExtractTemporaryFile('old_locales.txt');
    ExtractTemporaryFile('old_dirlist.txt');
    shortcut_list := TStringList.Create;
    shortcut_list.LoadFromFile(ExpandConstant('{tmp}\old_shortcutlist.txt'));
    file_list := TStringList.Create;
    file_list.LoadFromFile(ExpandConstant('{tmp}\old_filelist.txt'));
    locale_list := TStringList.Create;
    locale_list.LoadFromFile(ExpandConstant('{tmp}\old_locales.txt'));
    dir_list := TStringList.Create;
    dir_list.LoadFromFile(ExpandConstant('{tmp}\old_dirlist.txt'));
    itemsdone := 0;
    totalitems := file_list.Count + dir_list.Count + shortcut_list.Count + locale_list.Count + 1;
    // One extra for the start menu folder

    StartMenuFolder := ExpandConstant('{commonprograms}\Aegisub\');
    for i := 0 to shortcut_list.Count-1 do
    begin
      curname := StartMenuFolder + shortcut_list.Strings[i];
      page.SetText('Removing shortcuts', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove shortcut: ' + curname);
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;
    page.SetText('Removing Start menu folder', StartMenuFolder);
    page.SetProgress(itemsdone, totalitems);
    Log('Remove directory: ' + StartMenuFolder);
    if not RemoveDir(StartMenuFolder) then Log('* Directory deletion failed');
    itemsdone := itemsdone + 1;

    InstallFolder := ExpandConstant('{app}\');
    for i := 0 to file_list.Count-1 do
    begin
      curname := InstallFolder + file_list.Strings[i];
      page.SetText('Removing files', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove file: ' + curname);
      SetFileAttributes(curname, 128); // 128 = FILE_ATTRIBUTE_NORMAL
      if not DeleteFile(curname) then Log('* Deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to locale_list.Count-1 do
    begin
      curname := InstallFolder + locale_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory recursively: ' + curname);
      if not DelTree(curname, True, True, True) then Log('* Tree deletion failed');
      itemsdone := itemsdone + 1;
    end;

    for i := 0 to dir_list.Count-1 do
    begin
      curname := InstallFolder + dir_list.Strings[i];
      page.SetText('Removing folders', curname);
      page.SetProgress(itemsdone, totalitems);
      Log('Remove directory: ' + curname);
      if not RemoveDir(curname) then Log('* Directory deletion failed');
      itemsdone := itemsdone + 1;
    end;

    page.SetText('Uninstallation complete', '');
    page.SetProgress(totalitems, totalitems);

  finally
    shortcut_list.Free;
    file_list.Free;
    locale_list.Free;
    dir_list.Free;
    page.Hide;
  end;
end;


procedure CurStepChanged(CurStep: TSetupStep);
begin
	if CurStep = ssInstall then
	begin
	  if HasLegacyVersion then
	  begin
      MigrateStyleCatalogs;
      UninstallLegacyVersion;
    end
    CleanUpOldVersion;
	end;
end;

