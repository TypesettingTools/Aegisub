; Copyright (c) 2007, Niels Martin Hansen
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
; Website: http://aegisub.net/
; Contact: mailto:jiifurusu@gmail.com
;

#define MyAppName "Aegisub"
#define MyAppVerName "Aegisub 2.00 alpha r1611"
#define MyAppPublisher "Aegisub Team"
#define MyAppURL "http://aegisub.net/"
#define MyAppExeName "Aegisub.exe"

; Set these to 0 to make building the installer faster.
; Only intended for testing.
#define IncludeSpeller 1
#define IncludeThesaurus 1

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=true
OutputDir=output
OutputBaseFilename={#MyAppVerName} setup
Compression=lzma/ultra64
SolidCompression=true
MinVersion=0,5.0.2195
ShowLanguageDialog=no
LanguageDetectionMethod=none
WizardImageFile=welcome.bmp
WizardSmallImageFile=aegisub.bmp
AppCopyright=© 2005-2007 The Aegisub Team
PrivilegesRequired=poweruser
DisableProgramGroupPage=true
UsePreviousGroup=false
AlwaysShowComponentsList=true
AppVersion=2.00 alpha r1611
AppID={{24BC8B57-716C-444F-B46B-A3349B9164C5}
UninstallDisplayIcon={app}\Aegisub.exe

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; redist
Source: RuntimeTestLibrary\RuntimeTestLibrary.dll; Flags: dontcopy nocompression
Source: redist\vcredist_x86.exe; DestDir: {tmp}; Flags: nocompression deleteafterinstall; Components: main/runtime
; main
DestDir: {app}; Source: install\Aegisub.exe; Flags: ignoreversion; Components: main
DestDir: {app}; Source: install\Aegisub.pdb; Flags: ignoreversion; Components: main/pdb
DestDir: {app}; Source: install\changelog.txt; Flags: ignoreversion; Tasks: ; Components: main
DestDir: {app}; Source: install\license.txt; Flags: ignoreversion; Tasks: ; Languages: ; Components: main
DestDir: {app}; Source: install\aegisub-auto3.dll; Flags: ignoreversion; Components: main
; avisynth
DestDir: {app}; Source: redist\avisynth\devil.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: redist\avisynth\avisynth.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: redist\avisynth\DirectShowSource.dll; Flags: ignoreversion; Components: codec
; ffmpegsource
DestDir: {app}; Source: install\FFMpegSource.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: install\avcodec-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: install\avformat-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: install\avutil-49.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: install\postproc-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: install\swscale-0.dll; Flags: ignoreversion; Components: codec
; vsfilter
DestDir: {app}\csri; Source: install\csri\VSFilter.dll; Flags: ignoreversion; Components: codec/vsfilter
; auto4 main
DestDir: {app}\automation\include; Source: install\automation\include\utils.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\include; Source: install\automation\include\utils-auto4.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\include; Source: install\automation\include\unicode.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\include; Source: install\automation\include\karaskel.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\include; Source: install\automation\include\karaskel-auto4.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\include; Source: install\automation\include\cleantags.lua; Flags: ignoreversion; Components: auto/lua
DestDir: {app}\automation\autoload; Source: install\automation\autoload\kara-templater.lua; Flags: ignoreversion; Components: auto/lua
; auto4 samples
DestDir: {app}\automation\demos; Source: install\automation\demos\future-windy-blur.lua; Flags: ignoreversion; Components: auto/lua/samples
DestDir: {app}\automation\demos; Source: install\automation\demos\kara-templater-retime.ass; Flags: ignoreversion; Components: auto/lua/samples
DestDir: {app}\automation\autoload; Source: install\automation\autoload\macro-1-edgeblur.lua; Flags: ignoreversion; Components: auto/lua/samples
DestDir: {app}\automation\autoload; Source: install\automation\autoload\macro-2-mkfullwitdh.lua; Flags: ignoreversion; Components: auto/lua/samples
DestDir: {app}\automation\autoload; Source: install\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion; Components: auto/lua/samples
; auto3
DestDir: {app}\automation\include; Source: install\automation\include\utils.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\include; Source: install\automation\include\karaskel.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\include; Source: install\automation\include\karaskel-adv.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\include; Source: install\automation\include\karaskel-adv.lua; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\include; Source: install\automation\include\karaskel-base.lua; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\include; Source: install\automation\include\karaskel-base.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\auto3; Source: install\automation\auto3\line-per-syllable.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\auto3; Source: install\automation\auto3\multi-template.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\auto3; Source: install\automation\auto3\simple-k-replacer.auto3; Flags: ignoreversion; Components: auto/auto3
DestDir: {app}\automation\docs; Source: install\automation\docs\automation3.txt; Flags: ignoreversion; Components: auto/auto3
; dictionaries
#if IncludeSpeller != 0
Source: install\dictionaries\de_AT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion solidbreak; Components: dic/de_AT
Source: install\dictionaries\de_DE.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/de_DE
Source: install\dictionaries\de_DE.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/de_DE
Source: install\dictionaries\en_GB.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_GB
Source: install\dictionaries\en_GB.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_GB
Source: install\dictionaries\en_US.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_US
Source: install\dictionaries\en_US.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_US
Source: install\dictionaries\es_ES.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/es_ES
Source: install\dictionaries\es_ES.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/es_ES
Source: install\dictionaries\fr_FR.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/fr_FR
Source: install\dictionaries\fr_FR.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/fr_FR
Source: install\dictionaries\it_IT.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/it_IT
Source: install\dictionaries\it_IT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/it_IT
Source: install\dictionaries\nl_NL.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/nl_NL
Source: install\dictionaries\nl_NL.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/nl_NL
Source: install\dictionaries\pl_PL.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pl_PL
Source: install\dictionaries\pl_PL.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pl_PL
Source: install\dictionaries\pt_BR.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_BR
Source: install\dictionaries\pt_BR.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_BR
Source: install\dictionaries\pt_PT.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_PT
Source: install\dictionaries\pt_PT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_PT
Source: install\dictionaries\sk_SK.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sk_SK
Source: install\dictionaries\sk_SK.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sk_SK
Source: install\dictionaries\sl_SI.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sl_SI
Source: install\dictionaries\sl_SI.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sl_SI
Source: install\dictionaries\sv_SE.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sv_SE
Source: install\dictionaries\sv_SE.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sv_SE
#endif
; thesauri
#if IncludeThesaurus != 0
Source: install\dictionaries\th_de_DE.dat; DestDir: {app}\dictionaries; Flags: ignoreversion solidbreak; Components: th/de_DE
Source: install\dictionaries\th_de_DE.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/de_DE
Source: install\dictionaries\th_en_US.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/en_US
Source: install\dictionaries\th_en_US.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/en_US
Source: install\dictionaries\th_es_ES.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/es_ES
Source: install\dictionaries\th_es_ES.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/es_ES
Source: install\dictionaries\th_fr_FR.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/fr_FR
Source: install\dictionaries\th_fr_FR.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/fr_FR
Source: install\dictionaries\th_it_IT.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/it_IT
Source: install\dictionaries\th_it_IT.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/it_IT
#endif

[Icons]
Name: {commonprograms}\{#MyAppName}; Filename: {app}\Aegisub.exe; WorkingDir: {app}; IconIndex: 0; Components: main; Comment: Aegisub subtitle editor

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent
Filename: {tmp}\vcredist_x86.exe; StatusMsg: Installing runtime libraries...; Components: main/runtime; Parameters: /Q

[Components]
Name: main; Description: Aegisub; Types: compact full custom; Languages: ; Flags: fixed
Name: main/runtime; Description: Runtime libraries; Flags: fixed; Types: custom compact full; ExtraDiskSpaceRequired: 4630528; Check: CheckNeedRuntime
Name: main/pdb; Description: Debug database (helps diagnose crashes); Types: full
Name: codec; Description: Media formats support; Flags: fixed; Types: custom compact full
Name: codec/vsfilter; Description: VSFilter 2.38-aegisub; Types: compact full custom; Flags: fixed
Name: auto; Description: Automation 4 scripting support; Types: compact full
Name: auto/lua; Description: Lua; Types: compact full; Flags: checkablealone; Languages: 
Name: auto/lua/samples; Description: Lua sample scripts; Types: full
Name: auto/auto3; Description: Automation 3 backwards compatibility; Types: full
Name: help; Description: Help files (not written yet); Flags: fixed
Name: i18n; Description: Languages; Types: full custom compact; Flags: fixed
Name: i18n/english; Description: English (built in); Flags: fixed; Types: compact full custom
#if IncludeSpeller != 0
Name: dic; Description: Spell checker; Types: full
Name: dic/en_GB; Description: British English dictionary; Types: full
Name: dic/en_US; Description: American English dictionary; Types: full
Name: dic/nl_NL; Description: Dutch dictionary; Types: full
Name: dic/fr_FR; Description: French dictionary; Types: full
Name: dic/de_DE; Description: German dictionary; Types: full
Name: dic/de_AT; Description: Austrian German dictionary; Types: full
Name: dic/it_IT; Description: Italian dictionary; Types: full
Name: dic/es_ES; Description: Estonian dictionary; Types: full
Name: dic/pl_PL; Description: Polish dictionary; Types: full
Name: dic/pt_PT; Description: Portuguese dictionary; Types: full
Name: dic/pt_BR; Description: Brazilian Portuguese dictionary; Types: full
Name: dic/sk_SK; Description: Slovak dictionary; Types: full
Name: dic/sl_SI; Description: Slovenian dictionary; Types: full
Name: dic/sv_SE; Description: Swedish dictionary; Types: full
#endif
#if IncludeThesaurus != 0
Name: th; Description: Thesaurus; Types: full
Name: th/en_US; Description: American English thesaurus; Types: full
Name: th/es_ES; Description: Estonian thesaurus; Types: full
Name: th/fr_FR; Description: French thesaurus; Types: full
Name: th/de_DE; Description: German thesaurus; Types: full
Name: th/it_IT; Description: Italian thesaurus; Types: full
#endif

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel2=This will install {#MyAppVerName} on your computer.%n%n{#MyAppName} is covered by the GNU General Public License version 2. This means you may use the application for any purpose without charge, but that no warranties of any kind are given either.%n%nSee the {#MyAppName} website for information on obtaining the source code.

[Code]
var
	RuntimeLibChecked: Boolean;
	RuntimeLibInstalled: Boolean;

function LoadLibrary(lpFileName: string): LongInt; external 'LoadLibraryA@kernel32.dll stdcall';
function FreeLibrary(hModule: LongInt): Boolean; external 'FreeLibrary@kernel32.dll stdcall';

function CheckNeedRuntime: Boolean;
var
  LibHandle: LongInt;
begin
	if not RuntimeLibChecked then
	begin
		RuntimeLibInstalled := True;
		try
		  ExtractTemporaryFile('RuntimeTestLibrary.dll');
		  LibHandle := LoadLibrary(ExpandConstant('{tmp}') + '\RuntimeTestLibrary.dll');
		  if LibHandle = 0 then
		    RuntimeLibInstalled := False
		  else
		    FreeLibrary(LibHandle);
		except
			RuntimeLibInstalled := False;
		end;
		RuntimeLibChecked := True;
	end;
	Result := not RuntimeLibInstalled;
end;
