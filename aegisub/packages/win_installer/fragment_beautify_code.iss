[Code]
(*
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
*)


function SHAutoComplete(hWnd: Integer; dwFlags: DWORD): Integer;
external 'SHAutoComplete@shlwapi.dll stdcall delayload';
const
	SHACF_FILESYSTEM = $1;
	SHACF_FILESYS_ONLY = $10;
	SHACF_FILESYS_DIRS = $20;

procedure InitializeWizardBeautify;
var
  SmallBitmap: TFileStream;
begin
  // Thanks to ender for the following snippets

  // Fix bitmaps for small/large fonts
  try
    if WizardForm.WizardBitmapImage.Height < 386 then //use smaller image when not using Large Fonts
    begin
      ExtractTemporaryFile('welcome.bmp');
      SmallBitmap := TFileStream.Create(ExpandConstant('{tmp}\welcome.bmp'),fmOpenRead);
      try
        WizardForm.WizardBitmapImage.Bitmap.LoadFromStream(SmallBitmap);
        WizardForm.WizardBitmapImage2.Bitmap := WizardForm.WizardBitmapImage.Bitmap;
      finally
        SmallBitmap.Free;
      end;

      ExtractTemporaryFile('aegisub.bmp');
      SmallBitmap := TFileStream.Create(ExpandConstant('{tmp}\aegisub.bmp'),fmOpenRead);
      try
        WizardForm.WizardSmallBitmapImage.Bitmap.LoadFromStream(SmallBitmap);
      finally
        SmallBitmap.Free;
      end;
    end;
  except
    Log('Error loading bitmaps: ' + GetExceptionMessage);
  end;

  // Endow install dir edit box with autocomplete
  try
    SHAutoComplete(WizardForm.DirEdit.Handle, SHACF_FILESYSTEM);
  except
    Log('Could not add autocomplete to dir edit box');
  end;
end;

