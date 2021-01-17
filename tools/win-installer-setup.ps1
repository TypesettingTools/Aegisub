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

# DepCtrl
if (!(Test-Path DependencyControl)) {
	git clone https://github.com/TypesettingTools/DependencyControl.git
	Set-Location DependencyControl
	git checkout v0.6.3-alpha
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
if (!(Test-Path AviSynthPlus64)) {
	$avsReleases = Invoke-WebRequest "https://api.github.com/repos/AviSynth/AviSynthPlus/releases/latest" -UseBasicParsing | ConvertFrom-Json
	$avsUrl = $avsReleases.assets[0].browser_download_url
	Invoke-WebRequest $avsUrl -OutFile AviSynthPlus.7z -UseBasicParsing
	7z x AviSynthPlus.7z
	Rename-Item (Get-ChildItem -Filter "AviSynthPlus_*" -Directory) AviSynthPlus64
	Remove-Item AviSynthPlus.7z
}

# VSFilter
if (!(Test-Path VSFilter)) {
	$vsFilterDir = New-Item -ItemType Directory VSFilter
	Set-Location $vsFilterDir
	$vsFilterReleases = Invoke-WebRequest "https://api.github.com/repos/pinterf/xy-VSFilter/releases/latest" -UseBasicParsing | ConvertFrom-Json
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
	meson compile -C build
	Set-Location $DepsDir
}

# VC++ redistributable
if (!(Test-Path VC_redist)) {
	$redistDir = New-Item -ItemType Directory VC_redist
	Invoke-WebRequest https://aka.ms/vs/16/release/VC_redist.x64.exe -OutFile "$redistDir\VC_redist.x64.exe" -UseBasicParsing
}

# TODO dictionaries

# TODO localization

# Invoke InnoSetup
$IssUrl = Join-Path $InstallerDir "aegisub_depctrl.iss"
iscc $IssUrl
