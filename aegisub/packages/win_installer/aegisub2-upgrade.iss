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

#include "fragment_setupbase.iss"

[Setup]
OutputBaseFilename=Aegisub-2.1.8-upgrade
VersionInfoDescription=Aegisub 2.1.8 upgrade
DisableDirPage=yes


#include "fragment_mainprogram.iss"
#include "fragment_associations.iss"
#include "fragment_codecs.iss"
#include "fragment_automation.iss"
#include "fragment_translations.iss"


[InstallDelete]
; Usually this would be removed by the migration code, but that isn't used in upgrades so
; include the file deletion as a regular InstallDelete command
Type: files; Name: "{app}\csri\VSFilter.dll"; Components: codec/vsfilter


[Code]
#include "fragment_beautify_code.iss"

procedure InitializeWizard;
begin
  InitializeWizardBeautify;
end;

function InitializeSetup: Boolean;
var
  OldVersionString: string;
  OldVersionDir: string;
  OldExeSize: Integer;
begin
  // Check that we have an aegisub32.exe from 2.1.7 present and installation data from
  // 2.1.7 as well, otherwise don't allow setup to run.
  Result := False;
  try
    if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{24BC8B57-716C-444F-B46B-A3349B9164C5}_is1', 'DisplayVersion', OldVersionString) and
       RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{24BC8B57-716C-444F-B46B-A3349B9164C5}_is1', 'InstallLocation', OldVersionDir) and
       (OldVersionString = '2.1.7') and
       FileSize(OldVersionDir + '\aegisub32.exe', OldExeSize) and
       (OldExeSize = 5668864) and
       (GetMD5OfFile(OldVersionDir + '\aegisub32.exe') = 'c5e6c3e6511f4c293a12c980d8c4ca6e') then
    begin
      Result := True;
    end;
  except
    // Swallow
  end;
  
  if not Result then
  begin
    SuppressibleMsgBox(
      'Aegisub could not be upgraded because a supported version was not found.'#13#10#13#10 +
      'You must have Aegisub 2.1.7 installed to be able to use this upgrade installer.'#13#10 +
      'To install Aegisub 2.1.8, you must use the full version installer which can be downloaded from http://www.aegisub.org/.',
      mbError,
      MB_OK,
      IDOK);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
end;

