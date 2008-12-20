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
#define MyAppRevision "r2494"
#define MyAppVer "2.1.6 Release Preview"
#define MyAppPublisher "Aegisub Team"
#define MyAppURL "http://aegisub.net/"
#define MyAppExeName "Aegisub.exe"

; Set these to 0 to make building the installer faster.
; Only intended for testing.
#define IncludeSpeller 1
#define IncludeThesaurus 1
#define IncludePerl 1
#define IncludeFfmpeg 0


[Setup]
AppName={#MyAppName}
AppVerName={#MyAppName} {#MyAppVer} {#MyAppRevision}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=true
OutputDir=output
#ifndef Lite
OutputBaseFilename=aegisub-{#MyAppRevision}-setup
#else
OutputBaseFilename=aegisub-{#MyAppRevision}-lite-setup
#endif
Compression=lzma/ultra64
SolidCompression=true
MinVersion=0,5.0.2195
ShowLanguageDialog=no
LanguageDetectionMethod=none
WizardImageFile=welcome.bmp
WizardSmallImageFile=aegisub.bmp
AppCopyright=© 2005-2008 The Aegisub Team
PrivilegesRequired=poweruser
DisableProgramGroupPage=true
UsePreviousGroup=false
AlwaysShowComponentsList=true
AppVersion={#MyAppVer} {#MyAppRevision}
AppID={{24BC8B57-716C-444F-B46B-A3349B9164C5}
UninstallDisplayIcon={app}\Aegisub.exe

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Files]
; redist
;Source: RuntimeTestLibrary\RuntimeTestLibrary.dll; Flags: dontcopy nocompression
Source: redist\vcredist_x86.exe; Flags: dontcopy nocompression
; main
DestDir: {app}; Source: ..\bin\Aegisub.exe; Flags: ignoreversion; Components: main
DestDir: {app}; Source: ..\bin\Aegisub.pdb; Flags: ignoreversion; Components: main/pdb
DestDir: {app}; Source: ..\aegisub\changelog.txt; Flags: ignoreversion; Tasks: ; Components: main
DestDir: {app}; Source: license.txt; Flags: ignoreversion; Tasks: ; Languages: ; Components: main
DestDir: {app}; Source: ..\bin\aegisub-auto3.dll; Flags: ignoreversion; Components: main
DestDir: {app}; Source: ..\bin\OpenAL32.dll; Flags: ignoreversion; Components: main
; avisynth
DestDir: {app}; Source: ..\bin\devil.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\avisynth.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\DirectShowSource.dll; Flags: ignoreversion; Components: codec
; ffmpegsource
DestDir: {app}; Source: ..\bin\FFMS2.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\ffms2.html; Flags: ignoreversion; Components: codec
#if IncludeFfmpeg != 0
DestDir: {app}; Source: ..\bin\avcodec-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\avformat-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\avutil-49.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\postproc-51.dll; Flags: ignoreversion; Components: codec
DestDir: {app}; Source: ..\bin\swscale-0.dll; Flags: ignoreversion; Components: codec
#endif
; vsfilter
DestDir: {app}\csri; Source: ..\bin\csri\VSFilter.dll; Flags: ignoreversion; Components: codec/vsfilter
; auto4 main
DestDir: {app}\automation\include; Source: ..\automation\include\utils.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\utils-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\unicode.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel-auto4.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\cleantags.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
DestDir: {app}\automation\autoload; Source: ..\automation\autoload\kara-templater.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua; Attribs: readonly
; auto4 samples
DestDir: {app}\automation\demos; Source: ..\automation\demos\future-windy-blur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\demos; Source: ..\automation\tests\kara-templater-retime.ass; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: ..\automation\demos\macro-1-edgeblur.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: ..\automation\demos\macro-2-mkfullwitdh.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
DestDir: {app}\automation\autoload; Source: ..\automation\autoload\cleantags-autoload.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/lua/samples; Attribs: readonly
; perl
#if IncludePerl != 0
DestDir: {app}\automation\docs; Source: ..\automation\v4-docs\perl-api.txt; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\Aegisub.pm; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\Auto4Utils.pm; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\include\Aegisub; Source: ..\automation\include\Aegisub\PerlConsole.pm; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\include\Aegisub; Source: ..\automation\include\Aegisub\Script.pm; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\include\Aegisub; Source: ..\automation\include\Aegisub\Progress.pm; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl; Attribs: readonly
DestDir: {app}\automation\autoload; Source: ..\automation\demos\macro-1p-edgeblur.pl; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl/samples; Attribs: readonly
DestDir: {app}\automation\demos; Source: ..\automation\demos\perl-console.pl; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/perl/samples; Attribs: readonly
#endif
; auto3
DestDir: {app}\automation\include; Source: ..\automation\include\utils.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel-adv.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel-adv.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel-base.lua; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\include; Source: ..\automation\include\karaskel-base.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: ..\automation\auto3\line-per-syllable.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: ..\automation\auto3\multi-template.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\auto3; Source: ..\automation\auto3\simple-k-replacer.auto3; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
DestDir: {app}\automation\docs; Source: ..\automation\automation3.txt; Flags: ignoreversion overwritereadonly uninsremovereadonly; Components: auto/auto3; Attribs: readonly
; dictionaries
#if IncludeSpeller != 0
Source: ..\bin\dictionaries\en_GB.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_GB
Source: ..\bin\dictionaries\en_GB.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_GB
#ifndef Lite
Source: ..\bin\dictionaries\de_AT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion solidbreak; Components: dic/de_AT
Source: ..\bin\dictionaries\de_DE.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/de_DE
Source: ..\bin\dictionaries\de_DE.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/de_DE
Source: ..\bin\dictionaries\en_US.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_US
Source: ..\bin\dictionaries\en_US.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/en_US
Source: ..\bin\dictionaries\es_ES.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/es_ES
Source: ..\bin\dictionaries\es_ES.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/es_ES
Source: ..\bin\dictionaries\fr_FR.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/fr_FR
Source: ..\bin\dictionaries\fr_FR.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/fr_FR
Source: ..\bin\dictionaries\it_IT.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/it_IT
Source: ..\bin\dictionaries\it_IT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/it_IT
Source: ..\bin\dictionaries\nl_NL.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/nl_NL
Source: ..\bin\dictionaries\nl_NL.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/nl_NL
Source: ..\bin\dictionaries\pl_PL.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pl_PL
Source: ..\bin\dictionaries\pl_PL.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pl_PL
Source: ..\bin\dictionaries\pt_BR.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_BR
Source: ..\bin\dictionaries\pt_BR.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_BR
Source: ..\bin\dictionaries\pt_PT.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_PT
Source: ..\bin\dictionaries\pt_PT.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/pt_PT
Source: ..\bin\dictionaries\sk_SK.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sk_SK
Source: ..\bin\dictionaries\sk_SK.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sk_SK
Source: ..\bin\dictionaries\sl_SI.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sl_SI
Source: ..\bin\dictionaries\sl_SI.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sl_SI
Source: ..\bin\dictionaries\sv_SE.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sv_SE
Source: ..\bin\dictionaries\sv_SE.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/sv_SE
#endif
#endif
; thesauri
#if IncludeThesaurus != 0
#ifndef Lite
Source: ..\bin\dictionaries\th_de_DE.dat; DestDir: {app}\dictionaries; Flags: ignoreversion solidbreak; Components: th/de_DE
Source: ..\bin\dictionaries\th_de_DE.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/de_DE
Source: ..\bin\dictionaries\th_en_US.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/en_US
Source: ..\bin\dictionaries\th_en_US.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/en_US
Source: ..\bin\dictionaries\th_es_ES.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/es_ES
Source: ..\bin\dictionaries\th_es_ES.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/es_ES
Source: ..\bin\dictionaries\th_fr_FR.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/fr_FR
Source: ..\bin\dictionaries\th_fr_FR.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/fr_FR
Source: ..\bin\dictionaries\th_it_IT.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/it_IT
Source: ..\bin\dictionaries\th_it_IT.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/it_IT
#endif
#endif
; localization
Source: ..\po\ca.mo; DestDir: {app}\locale\ca; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/ca
Source: ..\po\wxstd-ca.mo; DestDir: {app}\locale\ca; DestName: wxstd.mo; Flags: ignoreversion; Components: i18n/ca
Source: ..\po\pt_BR.mo; DestDir: {app}\locale\pt_BR; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/pt_BR
Source: ..\po\wxstd-pt_BR.mo; DestDir: {app}\locale\pt_BR; DestName: wxstd.mo; Flags: ignoreversion; Components: i18n/pt_BR
Source: ..\po\es.mo; DestDir: {app}\locale\es; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/es
Source: ..\po\wxstd-es.mo; DestDir: {app}\locale\es; DestName: wxstd.mo; Flags: ignoreversion; Components: i18n/es
Source: ..\po\hu.mo; DestDir: {app}\locale\hu; DestName: aegisub.mo; Flags: ignoreversion; Components: i18n/hu
Source: ..\po\wxstd-hu.mo; DestDir: {app}\locale\hu; DestName: wxstd.mo; Flags: ignoreversion; Components: i18n/hu
; documentation
Source: ..\docs\output\*; DestDir: {app}\docs; Flags: ignoreversion recursesubdirs; Components: docs; Excludes: *svn
; ASSDraw3
Source: ..\bin\ASSDraw3.exe; DestDir: {app}; Flags: ignoreversion nocompression; Components: assdraw
Source: ..\bin\ASSDraw3.chm; DestDir: {app}; Flags: ignoreversion; Components: assdraw

[Icons]
Name: {commonprograms}\Aegisub\{#MyAppName}; Filename: {app}\Aegisub.exe; WorkingDir: {app}; IconIndex: 0; Components: main/icons; Comment: Aegisub subtitle editor
Name: {commonprograms}\Aegisub\ASSDraw3; Filename: {app}\ASSDraw3.exe; WorkingDir: {app}; IconIndex: 0; Components: main/icons; Flags: createonlyiffileexists; Comment: Aegisub subtitle editor
Name: {commonprograms}\Aegisub\Uninstall; Filename: {app}\unins000.exe; WorkingDir: {app}; IconIndex: 0; Components: main/icons; Comment: Uninstall Aegisub
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\Aegisub.exe; WorkingDir: {app}; IconIndex: 0; Components: main/qcklnch; Comment: Aegisub subtitle editor

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent
;Filename: {tmp}\vcredist_x86.exe; StatusMsg: Installing runtime libraries...; Components: main/runtime; Parameters: /Q

[Components]
Name: main; Description: Aegisub; Types: compact full custom; Languages: ; Flags: fixed
;Name: main/runtime; Description: Runtime libraries; Flags: fixed; Types: custom compact full; ExtraDiskSpaceRequired: 4630528
Name: main/pdb; Description: Debug database (helps diagnose crashes); Types: full
Name: main/icons; Description: Programs menu icons; Types: custom compact full
Name: main/qcklnch; Description: Quick launch icon; Types: custom compact full
Name: codec; Description: Media formats support; Flags: fixed; Types: custom compact full
Name: codec/vsfilter; Description: VSFilter 2.39 MPC-HC; Types: compact full custom; Flags: fixed
Name: auto; Description: Automation 4 scripting support; Types: compact full
Name: auto/lua; Description: Lua; Types: compact full; Flags: checkablealone; Languages: 
Name: auto/lua/samples; Description: Lua sample scripts; Types: full
#if IncludePerl != 0
Name: auto/perl; Description: Perl (requires a Perl 5.10 distribution); Types: compact full; Flags: checkablealone; Languages: 
Name: auto/perl/samples; Description: Perl sample scripts; Types: full; Flags: checkablealone; Languages:
#endif
Name: auto/auto3; Description: Automation 3 backwards compatibility; Types: full
Name: docs; Description: Documentation files; Types: custom compact full
Name: i18n; Description: Languages; Types: full custom compact; Flags: fixed
Name: i18n/english; Description: English (built in); Flags: fixed; Types: compact full custom
Name: i18n/pt_BR; Description: Brazilian Portuguese; Types: full
Name: i18n/ca; Description: Catalan; Types: full
Name: i18n/hu; Description: Hungarian; Types: full
Name: i18n/es; Description: Spanish; Types: full
#if IncludeSpeller != 0
Name: dic; Description: Spell checker; Types: full
Name: dic/en_GB; Description: British English dictionary; Types: full
#ifndef Lite
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
Name: dic/es_ES; Description: Spanish dictionary; Types: full
Name: dic/sv_SE; Description: Swedish dictionary; Types: full
#endif
#endif
#if IncludeThesaurus != 0
#ifndef Lite
Name: th; Description: Thesaurus; Types: full
Name: th/en_US; Description: American English thesaurus; Types: full
Name: th/fr_FR; Description: French thesaurus; Types: full
Name: th/de_DE; Description: German thesaurus; Types: full
Name: th/it_IT; Description: Italian thesaurus; Types: full
Name: th/es_ES; Description: Spanish thesaurus; Types: full
#endif
#endif
Name: assdraw; Description: ai-chan's ASSDraw3 for ASS vector drawing; Types: full

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel2=This will install {#MyAppName} {#MyAppVer} on your computer.%n%n{#MyAppName} is covered by the GNU General Public License version 2. This means you may use the application for any purpose without charge, but that no warranties of any kind are given either.%n%nSee the {#MyAppName} website for information on obtaining the source code.

[Code]
function LoadLibrary(lpFileName: string): LongInt; external 'LoadLibraryA@kernel32.dll stdcall';
function FreeLibrary(hModule: LongInt): Boolean; external 'FreeLibrary@kernel32.dll stdcall';

procedure CurStepChanged(CurStep: TSetupStep);
var
  LibHandle: LongInt;
  ExecResult: Integer;
  CustomPage: TOutputProgressWizardPage;
  temp1: String;
  temp2: String;
begin
	if CurStep = ssPostInstall then
	begin
		// LibHandle := LoadLibrary(ExpandConstant('{app}\csri\vsfilter.dll'));
    LibHandle := 0;
		if LibHandle = 0 then
		begin
		  temp2 := SetupMessage(msgInstallingLabel);
		  StringChangeEx(temp2,'[name]','{#MyAppName}',False);
		  CustomPage := CreateOutputProgressPage(SetupMessage(msgWizardInstalling),temp2);
		  CustomPage.SetText('Installing Visual C++ 2008 SP1 Runtimes... This might take a few minutes.','');
		  CustomPage.Show();
			ExtractTemporaryFile('vcredist_x86.exe');
			if not Exec(ExpandConstant('{tmp}\vcredist_x86.exe'), '/q:a /c:"VCREDI~3.EXE /q:a /c:""msiexec /i vcredist.msi /qn"" "', '', SW_SHOW, ewWaitUntilTerminated, ExecResult) then
			begin
				MsgBox('Installation of runtime libraries failed. Aegisub will probably not work. The error was: ' + SysErrorMessage(ExecResult), mbInformation, MB_OK);
			end;
			CustomPage.Hide();
		end
		else
			FreeLibrary(LibHandle);
	end;
end;
