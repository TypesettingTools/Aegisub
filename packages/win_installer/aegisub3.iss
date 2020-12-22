#define ARCH 64

#include "fragment_setupbase.iss"
#include "fragment_strings.iss"

[Setup]
AppID={{24BC8B57-716C-444F-B46B-A3349B9164C5}
DefaultDirName={pf}\Aegisub
PrivilegesRequired=poweruser
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

#include "fragment_mainprogram.iss"
#include "fragment_associations.iss"
#include "fragment_codecs.iss"
#include "fragment_automation.iss"
#include "fragment_translations.iss"
#include "fragment_spelling.iss"
#include "fragment_runtimes.iss"

[Code]
#include "fragment_shell_code.iss"
#include "fragment_migrate_code.iss"
#include "fragment_beautify_code.iss"

procedure InitializeWizard;
begin
  InitializeWizardBeautify;
end;

function InitializeSetup: Boolean;
begin
  Result := InitializeSetupMigration;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  Updates: String;
begin
  CurStepChangedMigration(CurStep);

  if CurStep = ssPostInstall then
  begin
    if IsTaskSelected('checkforupdates') then
      Updates := 'true'
    else
      Updates := 'false';

    SaveStringToFile(
      ExpandConstant('{app}\installer_config.json'),
      FmtMessage('{"App": {"Auto": {"Check For Updates": %1}, "First Start": false, "Language": "%2"}}', [
        Updates,
        ExpandConstant('{language}')]),
      False);
  end;
end;
