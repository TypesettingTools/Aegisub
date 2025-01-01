#define CURRENT_YEAR GetDateTimeString('yyyy', '', '');
#define BUILD_ROOT GetEnv('BUILD_ROOT')
#define SOURCE_ROOT GetEnv('SOURCE_ROOT')
#define INSTALLER_DIR SOURCE_ROOT + "\packages\win_installer"
#define DEPS_DIR BUILD_ROOT + "\installer-deps"

#include BUILD_ROOT + "\git_version.h"

[Setup]
AppName=Aegisub
AppVerName=Aegisub {#BUILD_GIT_VERSION_STRING}
AppVersion={#INSTALLER_VERSION}
AppPublisher=Aegisub Team
AppPublisherURL=http://aegisub.org/
AppSupportURL=https://github.com/TypesettingTools/Aegisub/issues
AppCopyright=2005-{#CURRENT_YEAR} The Aegisub Team
VersionInfoVersion={#INSTALLER_VERSION}
DefaultGroupName=Aegisub
AllowNoIcons=true
OutputDir={#BUILD_ROOT}
Compression=lzma/ultra64
SolidCompression=true
MinVersion=6.1sp1
ShowLanguageDialog=yes
LanguageDetectionMethod=none
DisableProgramGroupPage=yes
UsePreviousGroup=yes
UsePreviousSetupType=no
UsePreviousAppDir=yes
UsePreviousTasks=no
UsedUserAreasWarning=no
UninstallDisplayIcon={app}\aegisub.exe
; Default to a large welcome bitmap, suitable for large fonts
; The normal fonts version is selected by code below
WizardImageFile={#INSTALLER_DIR}\welcome-large.bmp
WizardSmallImageFile={#INSTALLER_DIR}\aegisub-large.bmp

OutputBaseFilename=Aegisub-{#BUILD_GIT_VERSION_STRING}
VersionInfoDescription=Aegisub {#BUILD_GIT_VERSION_STRING}

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
; The first language loaded affects the the initial window, English can't be alphabetized
Name: "bg"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "ca"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "fr_FR"; MessagesFile: "compiler:Languages\French.isl"
Name: "hu"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "ko"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "nl"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "pt_BR"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "pt_PT"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "tr"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "uk_UA"; MessagesFile: "compiler:Languages\Ukrainian.isl"
#ifdef UNOFFICIAL_LANGUAGES
; These languages are not shipped with InnoSetup by default and would need to be fetched from https://jrsoftware.org/files/istrans/
Name: "el"; MessagesFile: {#DEPS_DIR}\innosetup-langs\Greek.isl
Name: "eu"; MessagesFile: {#DEPS_DIR}\innosetup-langs\Basque.isl
Name: "gl"; MessagesFile: {#DEPS_DIR}\innosetup-langs\Galician.isl
Name: "id"; MessagesFile: {#DEPS_DIR}\innosetup-langs\Indonesian.isl
Name: "sr_RS"; MessagesFile: {#DEPS_DIR}\innosetup-langs\SerbianCyrillic.isl
Name: "sr_RS_latin"; MessagesFile: {#DEPS_DIR}\innosetup-langs\SerbianLatin.isl
Name: "zh_CN"; MessagesFile: {#DEPS_DIR}\innosetup-langs\ChineseSimplified.isl
Name: "zh_TW"; MessagesFile: {#DEPS_DIR}\innosetup-langs\ChineseTraditional.isl
#endif

[Files]
; small bitmaps (used by beautify code)
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\welcome.bmp
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\aegisub.bmp
; uninstall data (used by migration code)
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\legacy_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\legacy_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\legacy_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\legacy_shortcutlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\old_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\old_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\old_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: {#INSTALLER_DIR}\old_shortcutlist.txt
