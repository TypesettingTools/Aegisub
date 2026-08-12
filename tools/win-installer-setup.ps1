#!/usr/bin/env powershell

param (
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateNotNullOrEmpty()]
  [string]$BuildRoot,
  [Parameter(Position = 1, Mandatory = $true)]
  [ValidateNotNullOrEmpty()]
  [string]$SourceRoot,
  [Parameter(Position = 2)]
  [ValidateSet('x64', 'arm64')]
  [string]$Architecture = 'x64',
  [ValidateNotNullOrEmpty()]
  [string]$DepCtrlVersion = "0.8.1"
)

$ErrorActionPreference = 'Stop'

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

# DependencyControl
$DepCtrlDir = Join-Path $DepsDir "DependencyControl"

function Get-CachedDepCtrlVersion {
	param([Parameter(Mandatory = $true)][string]$Root)
	$module = Join-Path $Root "automation\include\l0\DependencyControl.moon"
	if (!(Test-Path -LiteralPath $module -PathType Leaf)) {
		return $null
	}
	$marker = Select-String -LiteralPath $module -Pattern 'version:\s*"([^"]+)".*--\s*@\{l0\.DependencyControl:version\}' |
		Select-Object -First 1
	if (!$marker) {
		return $null
	}
	$marker.Matches[0].Groups[1].Value
}

if ((Get-CachedDepCtrlVersion $DepCtrlDir) -ne $DepCtrlVersion) {
	$depCtrlUrl = "https://github.com/TypesettingTools/DependencyControl/releases/download/v$DepCtrlVersion/DependencyControl-v$DepCtrlVersion.zip"
	$depCtrlStagingDir = Join-Path $DepsDir ("DependencyControl-{0}" -f [guid]::NewGuid().ToString("N"))
	$depCtrlZip = Join-Path $depCtrlStagingDir "DependencyControl.zip"

	try {
		New-Item -ItemType Directory -Path $depCtrlStagingDir | Out-Null
		Invoke-WebRequest $depCtrlUrl -OutFile $depCtrlZip -UseBasicParsing
		7z x $depCtrlZip "-o$depCtrlStagingDir"
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to extract DependencyControl (7z exited with code $LASTEXITCODE)"
		}

		$stagedVersion = Get-CachedDepCtrlVersion $depCtrlStagingDir
		if (!$stagedVersion) {
			throw "DependencyControl archive did not contain a versioned automation\include\l0\DependencyControl.moon"
		}
		if ($stagedVersion -ne $DepCtrlVersion) {
			throw "DependencyControl archive contains version $stagedVersion, expected $DepCtrlVersion"
		}

		Remove-Item -LiteralPath $depCtrlZip
		if (Test-Path -LiteralPath $DepCtrlDir) {
			Remove-Item -LiteralPath $DepCtrlDir -Recurse -Force
		}
		Rename-Item -LiteralPath $depCtrlStagingDir -NewName (Split-Path $DepCtrlDir -Leaf)
		Write-Host "DependencyControl v$DepCtrlVersion has been downloaded to $DepCtrlDir"
	} finally {
		if (Test-Path -LiteralPath $depCtrlStagingDir) {
			Remove-Item -LiteralPath $depCtrlStagingDir -Recurse -Force -ErrorAction SilentlyContinue
		}
	}
} else {
	Write-Host "DependencyControl v$DepCtrlVersion already cached at $DepCtrlDir"
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

# VSFilter has no ARM64 build and must not be put in an ARM64 process.
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
$RedistName = "VC_redist.$Architecture.exe"
$RedistPath = Join-Path $RedistDir $RedistName
if (!(Test-Path $RedistPath)) {
	New-Item -ItemType Directory -Path $RedistDir -Force | Out-Null
	Invoke-WebRequest "https://aka.ms/vs/17/release/$RedistName" -OutFile $RedistPath -UseBasicParsing
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
if ($Architecture -eq 'arm64') {
	iscc /DARM64 $IssUrl
} else {
	iscc $IssUrl
}
if(!$?) { Exit $LASTEXITCODE }
