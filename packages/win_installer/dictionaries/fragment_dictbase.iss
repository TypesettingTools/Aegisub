[Setup]
AppID={{24BC8B57-716C-444F-B46B-A3349B9164C5}
AppName=Aegisub
AppVerName=Aegisub 3.1.0
AppVersion=3.1.0
AppPublisher=Aegisub Team
AppPublisherURL=http://www.aegisub.org/
AppSupportURL=http://forum.aegisub.org/
AppCopyright=© 2005-2014 The Aegisub Team
VersionInfoVersion=3.1.0
DefaultDirName={pf}\Aegisub
DefaultGroupName=Aegisub
AllowNoIcons=true
OutputDir=output
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

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel2=This will install {#LANGNAME} dictionaries for Aegisub 3.0 on your computer.

[Files]
; small bitmaps (used by beautify code)
DestDir: {tmp}; Flags: dontcopy; Source: welcome.bmp
DestDir: {tmp}; Flags: dontcopy; Source: aegisub.bmp

[Code]
#include "..\fragment_shell_code.iss"
#include "..\fragment_beautify_code.iss"

procedure InitializeWizard;
begin
  InitializeWizardBeautify;
end;
