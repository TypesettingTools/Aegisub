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
; Different ID than the regular Aegisub installer
AppID={{76FE2682-E393-4F70-9FFD-2401507FC748}
AppName=Aegisub
AppVerName=Aegisub 2.1.9
AppVersion=2.1.9
AppPublisher=Aegisub Team
AppPublisherURL=http://www.aegisub.org/
AppSupportURL=http://forum.aegisub.org/
AppCopyright=© 2005-2011 The Aegisub Team
VersionInfoVersion=2.1.9
VersionInfoDescription=Aegisub 2.1.9 portable package
DefaultDirName={userdesktop}\Aegisub portable
AllowNoIcons=true
Uninstallable=no
OutputDir=output
OutputBaseFilename=Aegisub-2.1.9-portable
Compression=lzma/ultra64
SolidCompression=true
MinVersion=0,5.0
ShowLanguageDialog=no
LanguageDetectionMethod=none
PrivilegesRequired=lowest
DisableProgramGroupPage=yes
; Default to a large welcome bitmap, suitable for large fonts
; The normal fonts version is selected by code below
WizardImageFile=welcome-large.bmp
WizardSmallImageFile=aegisub-large.bmp

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel1=Aegisub portable version
WelcomeLabel2=This will extract the portable version of Aegisub 2.1.9 to your computer.%n%nAegisub is covered by the GNU General Public License version 2. This means you may use the application for any purpose without charge, but that no warranties of any kind are given either.%n%nSee the Aegisub website for information on obtaining the source code.
; Select Dir page
SelectDirDesc=Where should Aegisub be extracted to?
SelectDirLabel3=Aegisub will be extracted into the following folder.
DiskSpaceWarning=You must have at least %1 KB of free space to extract Aegisub, but the selected drive only has %2 KB available.%n%nDo you want to continue anyway?
; Select Components page
SelectComponentsDesc=Which components should be extracted?
; Ready to Install page
WizardReady=Ready to extract
ReadyLabel1=Setup is now ready to extract Aegisub onto your computer.
ReadyLabel2a=Click Extract to continue, or click Back if you want to review or change any settings.
ReadyLabel2b=Click Extract to continue.
ButtonInstall=&Extract
; Preparing to Install page
WizardPreparing=Preparing to extract
PreparingDesc=Setup is preparing to extract Aegisub onto your computer.
; Installing page
WizardInstalling=Extracting
InstallingLabel=Please wait while Setup extracts Aegisub onto your computer.
; Setup Completed page
FinishedHeadingLabel=Completing the [name] Setup Wizard
FinishedLabelNoIcons=Setup has finished extracting Aegisub onto your computer.

[Files]
; small bitmaps (used by beautify code)
DestDir: {tmp}; Flags: dontcopy; Source: welcome.bmp
DestDir: {tmp}; Flags: dontcopy; Source: aegisub.bmp
; main
DestDir: {app}; Source: src\aegisub32.exe; Flags: ignoreversion solidbreak; Components: main
DestDir: {app}; Source: src\aegisub32.pdb; Flags: ignoreversion; Components: main/pdb
DestDir: {app}; Source: license.txt; Flags: ignoreversion; Components: main
DestDir: {app}; Source: portable_config.dat; DestName: config.dat; Flags: ignoreversion onlyifdoesntexist; Components: main
; runtimes
DestDir: {app}\Microsoft.VC90.CRT; Source: src\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest; Components: main/runtime
DestDir: {app}\Microsoft.VC90.CRT; Source: src\Microsoft.VC90.CRT\msvcm90.dll; Components: main/runtime
DestDir: {app}\Microsoft.VC90.CRT; Source: src\Microsoft.VC90.CRT\msvcp90.dll; Components: main/runtime
DestDir: {app}\Microsoft.VC90.CRT; Source: src\Microsoft.VC90.CRT\msvcr90.dll; Components: main/runtime
DestDir: {app}\Microsoft.VC90.OPENMP; Source: src\Microsoft.VC90.OPENMP\Microsoft.VC90.OpenMP.manifest; Components: main/runtime
DestDir: {app}\Microsoft.VC90.OPENMP; Source: src\Microsoft.VC90.OPENMP\vcomp90.dll; Components: main/runtime

[Components]
; Actual program
Name: main; Description: Aegisub; Types: compact full custom; Flags: fixed
Name: main/pdb; Description: Debug database (helps diagnose crashes); Types: full
Name: main/runtime; Description: Runtime libraries; Types: compact full custom; Flags: fixed

#include "fragment_codecs.iss"
#include "fragment_automation.iss"
#include "fragment_translations.iss"
#include "fragment_spelling.iss"
#include "fragment_docs.iss"


[Code]
#include "fragment_beautify_code.iss"

procedure InitializeWizard;
begin
  InitializeWizardBeautify;
end;




