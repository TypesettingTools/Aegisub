#!/usr/bin/env powershell

param (
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateNotNullOrEmpty()]
  [string]$BuildRoot,
  [Parameter(Position = 1, Mandatory = $true)]
  [ValidateNotNullOrEmpty()]
  [string]$SourceRoot,
  [Parameter(Position = 2, Mandatory = $true)]
  [ValidateSet('x64', 'arm64')]
  [string]$Architecture
)

$InstallerDir = Join-Path $SourceRoot "packages\win_installer" | Resolve-Path
$DepsDir = Join-Path $BuildRoot "installer-deps"
if (!(Test-Path $DepsDir)) {
	New-Item -ItemType Directory -Path $DepsDir | Out-Null
}

$Env:BUILD_ROOT = $BuildRoot
$Env:SOURCE_ROOT = $SourceRoot

$GitHeaders = @{}
if (Test-Path 'Env:GITHUB_TOKEN') {
	$GitHeaders = @{ 'Authorization' = 'Bearer ' + $Env:GITHUB_TOKEN }
}

# Yutils, luajson and the ffi-experiments libraries are no longer fetched: DependencyControl
# installs Yutils from its own feed and ships equivalents of json, BadMutex, PreciseTimer and
# DownloadManager, so it pulls each of them in on demand.

# DependencyControl
# Unlike a git clone, DepCtrl's release bundle already has the files arranged for Aegisub's automation directory layout.
$DepCtrlVersion = "v0.8.1"
$DepCtrlDir = Join-Path $DepsDir "DependencyControl"
if (!(Test-Path $DepCtrlDir)) {
	New-Item -ItemType Directory -Path $DepCtrlDir | Out-Null
	$depCtrlUrl = "https://github.com/TypesettingTools/DependencyControl/releases/download/$DepCtrlVersion/DependencyControl-$DepCtrlVersion.zip"
	$depCtrlZip = Join-Path $DepCtrlDir "DependencyControl.zip"
	Invoke-WebRequest $depCtrlUrl -OutFile $depCtrlZip -UseBasicParsing
	7z x $depCtrlZip "-o$DepCtrlDir"
	Remove-Item $depCtrlZip
}

# Avisynth
# $AviSynthDir = Join-Path $DepsDir "AviSynthPlus64"
# if (!(Test-Path $AviSynthDir)) {
# 	$avsReleases = Invoke-WebRequest "https://api.github.com/repos/AviSynth/AviSynthPlus/releases/latest" -Headers $GitHeaders -UseBasicParsing | ConvertFrom-Json
# 	$avsUrl = $avsReleases.assets[0].browser_download_url
# 	$avsArchive = Join-Path $DepsDir "AviSynthPlus.7z"
# 	Invoke-WebRequest $avsUrl -OutFile $avsArchive -UseBasicParsing
# 	7z x $avsArchive "-o$DepsDir"
# 	Rename-Item (Join-Path $DepsDir (Get-ChildItem -Path $DepsDir -Filter "AviSynthPlus_*" -Directory).Name) $AviSynthDir
# 	Remove-Item $avsArchive
# }

# VSFilter has no ARM64 build; libass remains the subtitle renderer there.
$VSFilterDir = Join-Path $DepsDir "VSFilter"
if ($Architecture -eq 'x64' -and !(Test-Path $VSFilterDir)) {
	New-Item -ItemType Directory -Path $VSFilterDir | Out-Null
	$vsFilterReleases = Invoke-WebRequest "https://api.github.com/repos/pinterf/xy-VSFilter/releases/latest" -Headers $GitHeaders -UseBasicParsing | ConvertFrom-Json
	$vsFilterUrl = $vsFilterReleases.assets[0].browser_download_url
	$vsFilterArchive = Join-Path $VSFilterDir "VSFilter.7z"
	Invoke-WebRequest $vsFilterUrl -OutFile $vsFilterArchive -UseBasicParsing
	7z x $vsFilterArchive "-o$VSFilterDir"
	Remove-Item $vsFilterArchive
}

# VC++ redistributable
$RedistDir = Join-Path $DepsDir "VC_redist"
if (!(Test-Path $RedistDir)) {
	New-Item -ItemType Directory -Path $RedistDir | Out-Null
	$redistUrl = "https://aka.ms/vc14/vc_redist.$Architecture.exe"
	Invoke-WebRequest $redistUrl -OutFile (Join-Path $RedistDir "VC_redist.$Architecture.exe") -UseBasicParsing
}

# Dictionaries
$DictionariesDir = Join-Path $DepsDir "dictionaries"
if (!(Test-Path $DictionariesDir)) {
	New-Item -ItemType Directory -Path $DictionariesDir | Out-Null
	Invoke-WebRequest https://raw.githubusercontent.com/TypesettingTools/Aegisub-dictionaries/master/dicts/en_US.aff -OutFile (Join-Path $DictionariesDir "en_US.aff") -UseBasicParsing
	Invoke-WebRequest https://raw.githubusercontent.com/TypesettingTools/Aegisub-dictionaries/master/dicts/en_US.dic -OutFile (Join-Path $DictionariesDir "en_US.dic") -UseBasicParsing
}

# Installer localization
$LangsDir = Join-Path $DepsDir "innosetup-langs"
if (!(Test-Path $LangsDir)) {
	New-Item -ItemType Directory -Path $LangsDir | Out-Null
	$LangBaseUrl = "https://raw.github.com/jrsoftware/issrc/is-6_7_3/Files/Languages/Unofficial"
	$Languages = @(
		'Greek', 'Basque', 'Galician', 'Indonesian',
		'SerbianCyrillic', 'SerbianLatin', 'ChineseSimplified', 'ChineseTraditional'
	)
	foreach ($lang in $Languages) {
		Invoke-WebRequest "$LangBaseUrl/$lang.isl" -OutFile (Join-Path $LangsDir "$lang.isl") -UseBasicParsing
	}
}

# Aegisub localization
meson compile -C $BuildRoot aegisub-gmo
if(!$?) { Exit $LASTEXITCODE }

# Invoke InnoSetup
$IssUrl = Join-Path $InstallerDir "aegisub_depctrl.iss"
$isccArgs = @()
if ($Architecture -eq 'arm64') { $isccArgs += '/DARM64' }
$isccArgs += $IssUrl
iscc @isccArgs
if(!$?) { Exit $LASTEXITCODE }
