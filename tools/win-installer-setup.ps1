#!/usr/bin/env powershell

param (
  [Parameter(Position = 0)]
  [string]$BuildRoot,
  [Parameter(Position = 1)]
  [string]$SourceRoot
)

$InstallerDir = Join-Path $SourceRoot "packages\win_installer" | Resolve-Path
$DepsDir = Join-Path $BuildRoot "installer-deps"
if (!(Test-Path $DepsDir)) {
	New-Item -ItemType Directory -Path $DepsDir
}

$Env:BUILD_ROOT = $BuildRoot
$Env:SOURCE_ROOT = $SourceRoot

Set-Location $DepsDir

$GitHeaders = @{}
if (Test-Path 'Env:GITHUB_TOKEN') {
	$GitHeaders = @{ 'Authorization' = 'Bearer ' + $Env:GITHUB_TOKEN }
}

# Yutils, luajson and the ffi-experiments libraries are no longer fetched: DependencyControl
# installs Yutils from its own feed and ships equivalents of json, BadMutex, PreciseTimer and
# DownloadManager, so it pulls each of them in on demand.

# DepCtrl
# Taken from the release bundle rather than a clone: the repository arranges its sources for its own
# tooling, while the bundle already carries Aegisub's automation directory layout.
$DepCtrlVersion = "v0.8.0"
if (!(Test-Path DependencyControl)) {
	$depCtrlDir = New-Item -ItemType Directory DependencyControl
	Set-Location $depCtrlDir
	$depCtrlUrl = "https://github.com/TypesettingTools/DependencyControl/releases/download/$DepCtrlVersion/DependencyControl-$DepCtrlVersion.zip"
	Invoke-WebRequest $depCtrlUrl -OutFile DependencyControl.zip -UseBasicParsing
	7z x DependencyControl.zip
	Remove-Item DependencyControl.zip
	Set-Location $DepsDir
}

# Avisynth
# if (!(Test-Path AviSynthPlus64)) {
# 	$avsReleases = Invoke-WebRequest "https://api.github.com/repos/AviSynth/AviSynthPlus/releases/latest" -Headers $GitHeaders -UseBasicParsing | ConvertFrom-Json
# 	$avsUrl = $avsReleases.assets[0].browser_download_url
# 	Invoke-WebRequest $avsUrl -OutFile AviSynthPlus.7z -UseBasicParsing
# 	7z x AviSynthPlus.7z
# 	Rename-Item (Get-ChildItem -Filter "AviSynthPlus_*" -Directory) AviSynthPlus64
# 	Remove-Item AviSynthPlus.7z
# }

# VSFilter
if (!(Test-Path VSFilter)) {
	$vsFilterDir = New-Item -ItemType Directory VSFilter
	Set-Location $vsFilterDir
	$vsFilterReleases = Invoke-WebRequest "https://api.github.com/repos/pinterf/xy-VSFilter/releases/latest" -Headers $GitHeaders -UseBasicParsing | ConvertFrom-Json
	$vsFilterUrl = $vsFilterReleases.assets[0].browser_download_url
	Invoke-WebRequest $vsFilterUrl -OutFile VSFilter.7z -UseBasicParsing
	7z x VSFilter.7z
	Remove-Item VSFilter.7z
	Set-Location $DepsDir
}

# VC++ redistributable
if (!(Test-Path VC_redist)) {
	$redistDir = New-Item -ItemType Directory VC_redist
	Invoke-WebRequest https://aka.ms/vs/17/release/VC_redist.x64.exe -OutFile "$redistDir\VC_redist.x64.exe" -UseBasicParsing
}

# Dictionaries
if (!(Test-Path dictionaries)) {
	New-Item -ItemType Directory dictionaries
	Invoke-WebRequest https://raw.githubusercontent.com/TypesettingTools/Aegisub-dictionaries/master/dicts/en_US.aff -OutFile dictionaries/en_US.aff -UseBasicParsing
	Invoke-WebRequest https://raw.githubusercontent.com/TypesettingTools/Aegisub-dictionaries/master/dicts/en_US.dic -OutFile dictionaries/en_US.dic -UseBasicParsing
}

# Installer localization
if (!(Test-Path innosetup-langs)) {
	New-Item -ItemType Directory innosetup-langs
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/Greek.isl -OutFile innosetup-langs/Greek.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/Basque.isl -OutFile innosetup-langs/Basque.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/Galician.isl -OutFile innosetup-langs/Galician.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/Indonesian.isl -OutFile innosetup-langs/Indonesian.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/SerbianCyrillic.isl -OutFile innosetup-langs/SerbianCyrillic.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/SerbianLatin.isl -OutFile innosetup-langs/SerbianLatin.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/ChineseSimplified.isl -OutFile innosetup-langs/ChineseSimplified.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial/ChineseTraditional.isl -OutFile innosetup-langs/ChineseTraditional.isl -UseBasicParsing
}

# Aegisub localization
Set-Location $BuildRoot
meson compile aegisub-gmo
if(!$?) { Exit $LASTEXITCODE }

# Invoke InnoSetup
$IssUrl = Join-Path $InstallerDir "aegisub_depctrl.iss"
iscc $IssUrl
if(!$?) { Exit $LASTEXITCODE }
