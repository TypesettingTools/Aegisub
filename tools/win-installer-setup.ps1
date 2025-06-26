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

# DepCtrl
if (!(Test-Path DependencyControl)) {
	git clone https://github.com/TypesettingTools/DependencyControl.git
	Set-Location DependencyControl
	git checkout v0.6.3-alpha
	Set-Location $DepsDir
}

# YUtils
if (!(Test-Path YUtils)) {
	git clone https://github.com/TypesettingTools/YUtils.git
}

# luajson
if (!(Test-Path luajson)) {
	git clone https://github.com/harningt/luajson.git
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

# ffi-experiments
if (!(Test-Path ffi-experiments)) {
	Get-Command "moonc" # check to ensure Moonscript is present
	git clone https://github.com/TypesettingTools/ffi-experiments.git
	Set-Location ffi-experiments
	meson build -Ddefault_library=static
	if(!$?) { Exit $LASTEXITCODE }
	meson compile -C build
	if(!$?) { Exit $LASTEXITCODE }
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
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Greek.isl -OutFile innosetup-langs/Greek.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Basque.isl -OutFile innosetup-langs/Basque.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Galician.isl -OutFile innosetup-langs/Galician.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/Indonesian.isl -OutFile innosetup-langs/Indonesian.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/SerbianCyrillic.isl -OutFile innosetup-langs/SerbianCyrillic.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/SerbianLatin.isl -OutFile innosetup-langs/SerbianLatin.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/ChineseSimplified.isl -OutFile innosetup-langs/ChineseSimplified.isl -UseBasicParsing
	Invoke-WebRequest https://raw.github.com/jrsoftware/issrc/main/Files/Languages/Unofficial/ChineseTraditional.isl -OutFile innosetup-langs/ChineseTraditional.isl -UseBasicParsing
}

# Aegisub localization
Set-Location $BuildRoot
meson compile aegisub-gmo
if(!$?) { Exit $LASTEXITCODE }

# Invoke InnoSetup
$IssUrl = Join-Path $InstallerDir "aegisub_depctrl.iss"
iscc $IssUrl
if(!$?) { Exit $LASTEXITCODE }
