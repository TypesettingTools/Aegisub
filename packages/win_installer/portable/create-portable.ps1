#!/usr/bin/env powershell

param (
    [Parameter(Position = 0)]
    [string]$BuildRoot,
    [Parameter(Position = 1)]
    [string]$SourceRoot
)

function Copy-New-Item {
    $SourceFilePath = $args[0]
    $DestinationFilePath = $args[1]
  
    If (-not (Test-Path $DestinationFilePath)) {
        New-Item -ItemType Directory -Path $DestinationFilePath -Force
    } 
    Copy-Item -Path $SourceFilePath -Destination $DestinationFilePath
}

function Copy-New-Items {
    $SourceFilePath = $args[0]
    $DestinationFilePath = $args[1]
  
    If (-not (Test-Path $DestinationFilePath)) {
        New-Item -ItemType Directory -Path $DestinationFilePath -Force
    } 
    Copy-Item -Path $SourceFilePath -Destination $DestinationFilePath -Recurse
}


Write-Output BUILD_ROOT=$BuildRoot
Write-Output SOURCE_ROOT=$SourceRoot
$InstallerDir = Join-Path $BuildRoot "install"
$InstallerDepsDir = Join-Path $BuildRoot "installer-deps"
$PortableOutputDir = Join-Path $BuildRoot "aegisub-portable"


Write-Output Goto building dir
Set-Location $BuildRoot


Write-Output 'Removing old temp dir'
Remove-Item -LiteralPath "$PortableOutputDir" -Force -Recurse
Remove-Item -LiteralPath "install" -Force -Recurse


Write-Output 'Make install'
meson install --no-rebuild --destdir $InstallerDir
Write-Output 'Gathering files'
Copy-New-Item $InstallerDir\bin\aegisub.exe  $PortableOutputDir
# TODO: check and remove freetype later
Copy-New-Item $InstallerDir\bin\freetype-6.dll  $PortableOutputDir

Write-Output 'Copying - translations'
Copy-New-Items "$InstallerDir\share\locale\*"  "$PortableOutputDir\locale" -Recurse
Write-Output 'Copying - codecs'
Write-Output 'Copying - codecs\Avisynth'
Copy-New-Item $InstallerDepsDir\AvisynthPlus64\x86-64\DevIL.dll  $PortableOutputDir
Copy-New-Item $InstallerDepsDir\AvisynthPlus64\x86-64\AviSynth.dll  $PortableOutputDir
Copy-New-Item $InstallerDepsDir\AvisynthPlus64\x86-64\plugins\DirectShowSource.dll  $PortableOutputDir
Write-Output 'Copying - codecs\VSFilter'
Copy-New-Item $InstallerDepsDir\VSFilter\x64\VSFilter.dll  $PortableOutputDir\csri
Write-Output 'Copying - runtimes\MS-CRT'
Copy-New-Item $InstallerDepsDir\VC_redist\VC_redist.x64.exe $PortableOutputDir\Microsoft.CRT

Write-Output 'Copying - automation'
Copy-New-Items "$InstallerDir\share\aegisub\automation\*"  "$PortableOutputDir\automation\"  -Recurse
Write-Output 'Copying - automation\DEPCTRL'
Copy-New-Items "$InstallerDepsDir\DependencyControl\modules\*"  "$PortableOutputDir\automation\include\l0\"  -Recurse
Copy-New-Items "$InstallerDepsDir\DependencyControl\macros\*"  "$PortableOutputDir\automation\autoload\"  -Recurse
Copy-New-Item $InstallerDepsDir\Yutils\src\Yutils.lua  $PortableOutputDir\automation\include
Copy-New-Items "$InstallerDepsDir\luajson\lua\*"  "$PortableOutputDir\automation\include\"  -Recurse

Copy-New-Item $InstallerDepsDir\ffi-experiments\build\requireffi\requireffi.lua  $PortableOutputDir\automation\include\requireffi
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\bad-mutex\BadMutex.dll  $PortableOutputDir\automation\include\BM\BadMutex
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\bad-mutex\BadMutex.lua  $PortableOutputDir\automation\include\BM
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\precise-timer\PreciseTimer.dll  $PortableOutputDir\automation\include\PT\PreciseTimer
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\precise-timer\PreciseTimer.lua  $PortableOutputDir\automation\include\PT
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\download-manager\DownloadManager.dll  $PortableOutputDir\automation\include\DM\DownloadManager
Copy-New-Item $InstallerDepsDir\ffi-experiments\build\download-manager\DownloadManager.lua  $PortableOutputDir\automation\include\DM

Write-Output 'Copying - portable-config'
Copy-New-Item $SourceRoot\packages\win_installer\portable\config.json  $PortableOutputDir


Write-Output 'Creating portable zip'
Remove-Item aegisub-portable-64.zip
7z a aegisub-portable-64.zip aegisub-portable\
