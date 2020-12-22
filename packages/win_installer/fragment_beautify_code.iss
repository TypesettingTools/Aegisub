[Code]
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
