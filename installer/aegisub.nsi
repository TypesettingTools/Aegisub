; Copyright (c) 2005, Fredrik Mellbin, Krunal Desai
; All rights reserved.
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
; Website: http://aegisub.cellosoft.com
; Contact: mailto:zeratul@cellosoft.com
;

;************************************************************************************
crccheck force
;************************************************************************************
;Macros

;--------------------------------
!define AEGISUB_VERSION "1.09"
!define NAME "Aegisub ${AEGISUB_VERSION}"
!define FNAME "Aegisub-${AEGISUB_VERSION}"
!define DISPLAYNAME "Aegisub ${AEGISUB_VERSION}"
!define VERSIONDATE "${__DATE__}"
!define AVISYNTH_INSTALLER "Avisynth_256.exe"
!define AVISYNTH_INSTALLER_SERVER "http://d.movax.org"
!define AVISYNTH_SOURCE "Avisynth_256_source.rar"
!define AVISYNTH_SOURCE_SERVER "http://files.cellosoft.com/zeratul"
;--------------------------------
;Version Information
  VIProductVersion "1.0.9.0"
  VIAddVersionKey  "ProductName" "Aegisub"
  VIAddVersionKey  "Comments" "Saving the world from bad typesetting."
  VIAddVersionKey  "CompanyName" "#aegisub"
  VIAddVersionKey  "FileDescription" "${NAME}"
  VIAddVersionKey  "FileVersion" "${AEGISUB_VERSION}"
  VIAddVersionKey  "LegalCopyright" "The Aegisub Group"
;--------------------------------
;Includes
   !include "MUI.nsh"
   !include "Sections.nsh"
   !include "WinMessages.nsh"
   !include "logiclib.nsh"
;************************************************************************************
;General Settings
  Name "${NAME}"

  SetCompressor /SOLID /FINAL LZMA

  Outfile "${FNAME}.exe"
  InstallDirRegKey HKLM "Software\Aegisub\info" "InstallDir"
  InstallDir "$PROGRAMFILES\Aegisub"

  var startmenudir
  var prevname
  var prevdir
  var prevver
  var vsfilter_installed
  var aegisub_source_installed
  var help_installed
;************************************************************************************
;Modern UI Configuration
  !define MUI_COMPONENTSPAGE_CHECKBITMAP "C:\Program Files\NSIS\Contrib\Graphics\Checks\modern.bmp"
  !define MUI_COMPONENTSPAGE_SMALLDESC
  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_UNFINISHPAGE_NOAUTOCLOSE
  !define MUI_ABORTWARNING
  !define MUI_UNABORTWARNING
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "aegisub.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "Aegisub"
  XPstyle on
;************************************************************************************
;Pages
  !insertmacro MUI_PAGE_WELCOME
  !define MUI_LICENSEPAGE_CHECKBOX
  !define MUI_LICENSEPAGE_CHECKBOX_TEXT "I agree."
  !insertmacro MUI_PAGE_LICENSE license.txt
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_STARTMENU 1 $startmenudir
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
;************************************************************************************
;Languages
   !insertmacro MUI_LANGUAGE "English"
;************************************************************************************
;Language Strings
   LangString DESC_AEGISUB_GROUP ${LANG_ENGLISH} \
   "Aegisub ${AEGISUB_VERSION} Program files."
   LangString DESC_AVISYNTH_GROUP ${LANG_ENGLISH} \
   "Download the avisynth installer or source."
   LangString DESC_AEGISUB_SECTION ${LANG_ENGLISH} \
   "${NAME} main program files."
   LangString DESC_AEGISUB_SOURCE_SECTION ${LANG_ENGLISH} \
   "${NAME} source files."
   LangString DESC_AEGISUB_HELP_SECTION ${LANG_ENGLISH} \
   "Aegisub help files."
   LangString DESC_VSFILTER_SECTION ${LANG_ENGLISH} \
   "Places a copy of VSFilter 2.37 in the Aegisub directory."
   LangString DESC_AVISYNTH_INSTALLER_SECTION ${LANG_ENGLISH} \
   "Downloads the Avisynth 2.5.6a installer and executes it."
   LangString DESC_AVISYNTH_SOURCE_SECTION ${LANG_ENGLISH} \
   "Downloads the Avisynth 2.5.6a source and places it on the desktop."
;************************************************************************************
reservefile "${NSISDIR}\Plugins\Userinfo.dll"
reservefile "${NSISDIR}\Plugins\Nsisdl.dll"

!macro checkrightspoweruser
 UserInfo::GetAccountType
 pop $R0

 ${if} $R0 != "Admin"
 ${andif} $R0 != "Power"

  MessageBox MB_OK|MB_ICONSTOP "This program can only be run by power users or higher." IDOK
  abort
 ${endif}
!macroend

!macro checkrightsnormaluser
 UserInfo::GetAccountType
 pop $R0

 ${if} $R0 != "Admin"
 ${andif} $R0 != "Power"
 ${andif} $R0 != "Normal"

  MessageBox MB_OK|MB_ICONSTOP "This program can only be run by normal users or higher." IDOK
  abort
 ${endif}
!macroend
;************************************************************************************
;Design / Text
   XPStyle on
   BrandingText "${NAME} | ${VERSIONDATE}"
   ShowInstDetails hide
;************************************************************************************
;Functions
;************************************************************************************
Function .onInit
  Call GetWindowsVersion
  Pop $R0

  ${if} $R0 != "2000"
  ${andif} $R0 != "XP"
  ${andif} $R0 != "2003"
  MessageBox MB_OK|MB_ICONSTOP "Only windows 2000/XP/2003 is supported by this installer."
  quit
  ${endif}
  !insertmacro checkrightspoweruser
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "aegisinstaller") i .r1 ?e'
  Pop $R0
  ${unless} $R0 = 0
  MessageBox MB_OK|MB_ICONEXCLAMATION "Installer already running."
  Abort
  ${endunless}

  ClearErrors
  ReadRegStr $prevver HKLM "Software\Aegisub\info" "InstVer"
  IfErrors lbl_new_installation
    ReadRegStr $prevdir HKLM "Software\Aegisub\info" "InstallDir"
    ReadRegStr $prevname HKLM "Software\Aegisub\info" "InstName"

    ${if} $prevdir != ""
      StrCpy $INSTDIR $prevdir
    ${endif}

    MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION "Previous Aegisub install detected (Version $prevver). Uninstall? (No will overwrite existing files, Cancel will exit.)" IDYES lbl_uninstall_previous IDNO lbl_new_installation
      quit
    lbl_uninstall_previous:
      CopyFiles "$prevdir\uninstall.exe" $TEMP
      ExecWait '"$TEMP\uninstall.exe" _?=$prevdir'
      Delete "$TEMP\uninstall.exe"
  lbl_new_installation:
FunctionEnd

Function un.onInit
  !insertmacro checkrightspoweruser
FunctionEnd

;************************************************************************************
;Installer Sections
;************************************************************************************
SectionGroup /e "!Aegisub Program Files" AEGISUB_GROUP

Section "Aegisub" AEGISUB_SECTION
  SectionIn RO

  SetOutPath $INSTDIR

  File "Aegisub.exe"
  File "Aegisub Site.url"
  File "changelog.txt"
  File /r "locale"

  File "msvcp71.dll"
  File "msvcr71.dll"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  File /r "automation"

!insertmacro MUI_STARTMENU_WRITE_BEGIN 1
  SetOutPath $smprograms\$startmenudir
  CreateShortCut "$smprograms\$startmenudir\Aegisub.lnk" "$INSTDIR\aegisub.exe"
  CreateShortCut "$smprograms\$startmenudir\Aegisub Site.lnk" "$INSTDIR\Aegisub Site.url"
  CreateShortCut "$smprograms\$startmenudir\Changelog.lnk" "$INSTDIR\changelog.txt"
  CreateShortCut "$smprograms\$startmenudir\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
!insertmacro MUI_STARTMENU_WRITE_END

  WriteRegDWORD HKLM "Software\Aegisub" "aegisub" "1"
  WriteRegStr HKLM "Software\Aegisub\info" "InstName" "${NAME}"
  WriteRegStr HKLM "Software\Aegisub\info" "InstVer" "${AEGISUB_VERSION}"
  WriteRegStr HKLM "Software\Aegisub\info" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Aegisub\info" "StartMenuDir" "$startmenudir"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayName" "${DISPLAYNAME} (Remove Only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayIcon" "$INSTDIR\aegisub.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "NoRepair" "1"
SectionEnd

Section "Aegisub Help" AEGISUB_HELP_SECTION
SetOutPath $INSTDIR
   File "Aegisub.chm"
   WriteRegDWORD HKLM "Software\Aegisub" "aegisub_help" "1"

!insertmacro MUI_STARTMENU_WRITE_BEGIN 1
  SetOutPath $smprograms\$startmenudir
  CreateShortCut "$smprograms\$startmenudir\Aegisub Help.lnk" "$INSTDIR\aegisub.chm"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "VSFilter 2.37" VSFILTER_SECTION
SetoutPath $INSTDIR
   File "vsfilter.dll"
   WriteRegDWORD HKLM "Software\Aegisub" "aegisub_vsfilter" "1"
SectionEnd

Section /o "Aegisub Source" AEGISUB_SOURCE_SECTION
SetoutPath $INSTDIR
   File /r "source"
   WriteRegDWORD HKLM "Software\Aegisub" "aegisub_source" "1"
SectionEnd
SectionGroupEnd

SectionGroup /e "Avisynth" AVISYNTH_GROUP

Section /o "Avisynth 2.5.6a" AVISYNTH_INSTALLER_SECTION
SetOutPath $INSTDIR

   IfFileExists "$DESKTOP\${AVISYNTH_INSTALLER}" lbl_avisynth_end

   ReadRegStr $R2 HKLM "Software\Avisynth" ""
   ${if} $R2 != ""
    MessageBox MB_ICONQUESTION|MB_YESNO "Avisynth is already installed. Would you like to install over the current version? (required if your version is older than 2.5.0)" IDNO lbl_avisynth_end
   ${endif}

   GetTempFilename $R1 $TEMP

   lbl_avisynth_dl:
    Delete $R1
    NSISdl::download /TRANSLATE "Downloading ${AVISYNTH_INSTALLER}" "Connecting..." "second" "minute" "hour" "s" "%dkB (%d%%) of %dkB @ %d.%01dkB/s" "(%d %s%s remaining)" "${AVISYNTH_INSTALLER_SERVER}/${AVISYNTH_INSTALLER}" "$R1"
    Pop $R0 ;Get the return value
    StrCmp $R0 "success" lbl_avisynth_success lbl_avisynth_fail
   lbl_avisynth_fail:
    MessageBox MB_ABORTRETRYIGNORE "Download failed: $R0" IDIGNORE lbl_avisynth_ignore IDRETRY lbl_avisynth_dl 
    goto lbl_avisynth_abort
   lbl_avisynth_success:
    Rename $R1 "$DESKTOP\${AVISYNTH_INSTALLER}"

   lbl_avisynth_install:
    ExecWait "$DESKTOP\${AVISYNTH_INSTALLER}"

    ReadRegStr $R2 HKLM "Software\Avisynth" ""
    ${if} $R2 == ""
     MessageBox MB_ABORTRETRYIGNORE "Avisynth install either failed or was cancelled." IDRETRY lbl_avisynth_install IDIGNORE lbl_avisynth_end
     goto lbl_avisynth_abort
    ${endif}

    goto lbl_avisynth_end

   lbl_avisynth_ignore:
    Delete $R1
    goto lbl_avisynth_end

   lbl_avisynth_abort:
    Delete $R1
    quit

   lbl_avisynth_end:

SectionEnd

Section /o "Avisynth 2.5.6a Source" AVISYNTH_SOURCE_SECTION
SetOutPath $INSTDIR

   IfFileExists "$DESKTOP\${AVISYNTH_SOURCE}" lbl_avisynth_source_end

   GetTempFilename $R1 $TEMP

   lbl_avisynth_source_dl:
    Delete $R1
    NSISdl::download /TRANSLATE "Downloading ${AVISYNTH_SOURCE}" "Connecting..." "second" "minute" "hour" "s" "%dkB (%d%%) of %dkB @ %d.%01dkB/s" "(%d %s%s remaining)" "${AVISYNTH_SOURCE_SERVER}/avisynth_src.rar" "$R1"
    Pop $R0 ;Get the return value
    StrCmp $R0 "success" lbl_avisynth_source_success lbl_avisynth_source_fail
   lbl_avisynth_source_fail:
    MessageBox MB_ABORTRETRYIGNORE "Download failed: $R0" IDIGNORE lbl_avisynth_source_ignore IDRETRY lbl_avisynth_source_dl 
    goto lbl_avisynth_source_abort
   lbl_avisynth_source_success:
    Rename $R1 "$DESKTOP\${AVISYNTH_SOURCE}"
    goto lbl_avisynth_source_end

   lbl_avisynth_source_ignore:
    Delete $R1
    goto lbl_avisynth_source_end

   lbl_avisynth_source_abort:
    Delete $R1
    quit

   lbl_avisynth_source_end:
SectionEnd

SectionGroupEnd
;************************************************************************************
Section "Uninstall"
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "aegisun") i .r1 ?e'
  Pop $R0
  ${unless} $R0 = 0
  MessageBox MB_OK|MB_ICONEXCLAMATION "Uninstaller already running."
  Abort
  ${endunless}

!insertmacro checkrightspoweruser

SetOutPath $INSTDIR
Delete "$INSTDIR\Aegisub.exe"
Delete "$INSTDIR\Aegisub Site.url"
Delete "$INSTDIR\changelog.txt"
Delete "$INSTDIR\uninstall.exe"
Delete "$INSTDIR\msvcp71.dll"
Delete "$INSTDIR\msvcr71.dll"

RMDir /r "$INSTDIR\locale"
RMDir "$INSTDIR\autosave"
RMDir "$INSTDIR\autoback"

;automation
  Delete "$INSTDIR\automation\automation-lua.txt"

;include
  Delete "$INSTDIR\automation\include\readme.txt"
  Delete "$INSTDIR\automation\include\utils.lua"
  Delete "$INSTDIR\automation\include\karaskel.lua"
  Delete "$INSTDIR\automation\include\karaskel-adv.lua"
  RMDir "$INSTDIR\automation\include"

;factorybrew
  RMDir /r "$INSTDIR\automation\factorybrew"

;demos
  RMDir /r "$INSTDIR\automation\demos"

  RMDir "$INSTDIR\automation"

;automation end

IfFileExists "$INSTDIR\catalog\*" "" lbl_empty_catalog
 MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to keep your styles catalog?" IDYES lbl_empty_catalog
  Delete "$INSTDIR\catalog\*"
lbl_empty_catalog:
RMDir "$INSTDIR\catalog"

ReadRegDWORD $vsfilter_installed HKLM "Software\Aegisub" "aegisub_vsfilter"
ReadRegDWORD $aegisub_source_installed HKLM "Software\Aegisub" "aegisub_source"
ReadRegDWORD $help_installed HKLM "Software\Aegisub" "aegisub_help"
ReadRegStr $startmenudir HKLM "Software\Aegisub\info" "StartMenuDir"

${if} $aegisub_source_installed = 1
  MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to keep the source directory?" IDYES lbl_keep_aegisub_source
    RMDir /r "$INSTDIR\source"
  lbl_keep_aegisub_source:
${endif}

${if} $vsfilter_installed = 1
  Delete "$INSTDIR\vsfilter.dll"
${endif}

${if} $help_installed = 1
  Delete "$INSTDIR\aegisub.chm"
  Delete "$SMPROGRAMS\$startmenudir\Aegisub Help.lnk"
${endif}

DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"
DeleteRegKey HKLM "Software\Aegisub"

;backup config
IfFileExists "$INSTDIR\config.dat" "" lbl_no_config_end

MessageBox MB_YESNO "Would you like to save your Aegisub configuration?" IDYES lbl_no_config_end
  Delete "$INSTDIR\config.dat"
lbl_no_config_end:

${unless} $startmenudir == ""
  Delete "$SMPROGRAMS\$startmenudir\Aegisub.lnk"
  Delete "$SMPROGRAMS\$startmenudir\Uninstall.lnk"
  Delete "$SMPROGRAMS\$startmenudir\Aegisub Site.lnk"
  Delete "$SMPROGRAMS\$startmenudir\Changelog.lnk"
  RMDir "$SMPROGRAMS\$startmenudir"
${endunless}

  SetOutPath $TEMP
  RMDir "$INSTDIR"
SectionEnd
;************************************************************************************
Function GetWindowsVersion
 Push $R0
 Push $R1

 ClearErrors

 ReadRegStr $R0 HKLM \
 "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

 IfErrors 0 lbl_winnt

 ; we are not NT
 ReadRegStr $R0 HKLM \
 "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber

 StrCpy $R1 $R0 1
 StrCmp $R1 '4' 0 lbl_error

 StrCpy $R1 $R0 3

 StrCmp $R1 '4.0' lbl_win32_95
 StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98

 lbl_win32_95:
  StrCpy $R0 '95'
 Goto lbl_done

 lbl_win32_98:
  StrCpy $R0 '98'
 Goto lbl_done

 lbl_win32_ME:
  StrCpy $R0 'ME'
 Goto lbl_done

 lbl_winnt:

 StrCpy $R1 $R0 1

 StrCmp $R1 '3' lbl_winnt_x
 StrCmp $R1 '4' lbl_winnt_x

 StrCpy $R1 $R0 3

 StrCmp $R1 '5.0' lbl_winnt_2000
 StrCmp $R1 '5.1' lbl_winnt_XP
 StrCmp $R1 '5.2' lbl_winnt_2003 lbl_error

 lbl_winnt_x:
  StrCpy $R0 "NT $R0" 6
 Goto lbl_done

 lbl_winnt_2000:
  Strcpy $R0 '2000'
 Goto lbl_done

 lbl_winnt_XP:
  Strcpy $R0 'XP'
 Goto lbl_done

 lbl_winnt_2003:
  Strcpy $R0 '2003'
 Goto lbl_done

 lbl_error:
  Strcpy $R0 ''
 lbl_done:

 Pop $R1
 Exch $R0

FunctionEnd
;************************************************************************************
;Descriptions
   !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
      !insertmacro MUI_DESCRIPTION_TEXT ${AEGISUB_GROUP} $(DESC_AEGISUB_GROUP)
      !insertmacro MUI_DESCRIPTION_TEXT ${AVISYNTH_GROUP} $(DESC_AVISYNTH_GROUP)
      !insertmacro MUI_DESCRIPTION_TEXT ${AEGISUB_SECTION} $(DESC_AEGISUB_SECTION)
      !insertmacro MUI_DESCRIPTION_TEXT ${AEGISUB_SOURCE_SECTION} $(DESC_AEGISUB_SOURCE_SECTION)
      !insertmacro MUI_DESCRIPTION_TEXT ${AEGISUB_HELP_SECTION} $(DESC_AEGISUB_HELP_SECTION)
      !insertmacro MUI_DESCRIPTION_TEXT ${VSFILTER_SECTION} $(DESC_VSFILTER_SECTION)
      !insertmacro MUI_DESCRIPTION_TEXT ${AVISYNTH_INSTALLER_SECTION} $(DESC_AVISYNTH_INSTALLER_SECTION)
      !insertmacro MUI_DESCRIPTION_TEXT ${AVISYNTH_SOURCE_SECTION} $(DESC_AVISYNTH_SOURCE_SECTION)
   !insertmacro MUI_FUNCTION_DESCRIPTION_END
;*****************EOF*********EOF***********EOF**************EOF*********EOF**********