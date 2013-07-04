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

#define VERSION "3.0.4"

[Setup]
AppName=Aegisub
AppVerName=Aegisub {#VERSION}
AppVersion={#VERSION}
AppPublisher=Aegisub Team
AppPublisherURL=http://www.aegisub.org/
AppSupportURL=http://forum.aegisub.org/
AppCopyright=© 2005-2013 The Aegisub Team
VersionInfoVersion={#VERSION}
DefaultGroupName=Aegisub
AllowNoIcons=true
OutputDir=output
Compression=lzma/ultra64
SolidCompression=true
MinVersion=0,5.0
ShowLanguageDialog=no
LanguageDetectionMethod=none
DisableProgramGroupPage=yes
UsePreviousGroup=yes
UsePreviousSetupType=no
UsePreviousAppDir=yes
UsePreviousTasks=no
UninstallDisplayIcon={app}\aegisub{#ARCH}.exe
; Default to a large welcome bitmap, suitable for large fonts
; The normal fonts version is selected by code below
WizardImageFile=welcome-large.bmp
WizardSmallImageFile=aegisub-large.bmp

OutputBaseFilename=Aegisub-{#VERSION}-{#ARCH}
VersionInfoDescription=Aegisub {#VERSION} {#ARCH}

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Messages]
; Replacement for License page, no need to bother the user with legal mumbo-jumbo
WelcomeLabel2=This will install Aegisub {#VERSION} on your computer.%n%nAegisub is covered by the GNU General Public License version 2. This means you may use the application for any purpose without charge, but that no warranties of any kind are given either.%n%nSee the Aegisub website for information on obtaining the source code.

[Files]
; small bitmaps (used by beautify code)
DestDir: {tmp}; Flags: dontcopy; Source: welcome.bmp
DestDir: {tmp}; Flags: dontcopy; Source: aegisub.bmp
; uninstall data (used by migration code)
DestDir: {tmp}; Flags: dontcopy; Source: legacy_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: legacy_shortcutlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_filelist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_dirlist.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_locales.txt
DestDir: {tmp}; Flags: dontcopy; Source: old_shortcutlist.txt



