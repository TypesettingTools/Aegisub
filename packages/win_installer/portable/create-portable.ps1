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

$ErrorActionPreference = 'Stop'

function Copy-ToDirectory {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][string]$Destination,
        [switch]$Recurse
    )
    New-Item -ItemType Directory -Path $Destination -Force | Out-Null
    Copy-Item -Path $Path -Destination $Destination -Recurse:$Recurse -Force
}

# Keep in sync with the number of Write-Step calls below.
$script:stepNum = 0
$script:stepTotal = 11

# Report progress both via an interactive bar and a textual trail for CI logs.
function Write-Step {
    param([Parameter(Mandatory)][string]$Status)
    $script:stepNum++
    Write-Progress -Activity 'Creating portable Aegisub' -Status $Status -PercentComplete (100 * $script:stepNum / $script:stepTotal)
    Write-Host "[$script:stepNum/$script:stepTotal] $Status"
}

Write-Host "BUILD_ROOT=$BuildRoot"
Write-Host "SOURCE_ROOT=$SourceRoot"
$InstallerDir = Join-Path $BuildRoot "install"
$InstallerDepsDir = Join-Path $BuildRoot "installer-deps"
$PortableOutputDir = Join-Path $BuildRoot "aegisub-portable"
$PortableZipPath = Join-Path $BuildRoot "aegisub-portable-$Architecture.zip"

Write-Step 'Removing previous output'
Remove-Item -LiteralPath $PortableOutputDir -Force -Recurse -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $InstallerDir -Force -Recurse -ErrorAction SilentlyContinue

Write-Step 'Installing build output'
meson install -C $BuildRoot --no-rebuild --destdir $InstallerDir
if ($LASTEXITCODE -ne 0) { throw "meson install failed (exit $LASTEXITCODE)" }

Write-Step 'Copying executable'
Copy-ToDirectory $InstallerDir\bin\aegisub.exe  $PortableOutputDir

Write-Step 'Copying translations'
Copy-ToDirectory "$InstallerDir\share\locale\*"  "$PortableOutputDir\locale" -Recurse

Write-Step 'Copying dictionaries'
Copy-ToDirectory $InstallerDepsDir\dictionaries\en_US.aff  $PortableOutputDir\dictionaries
Copy-ToDirectory $InstallerDepsDir\dictionaries\en_US.dic  $PortableOutputDir\dictionaries

# Write-Step 'AviSynth'
# Copy-ToDirectory $InstallerDepsDir\AvisynthPlus64\x64\Output\system\DevIL.dll  $PortableOutputDir
# Copy-ToDirectory $InstallerDepsDir\AvisynthPlus64\x64\Output\AviSynth.dll  $PortableOutputDir
# Copy-ToDirectory $InstallerDepsDir\AvisynthPlus64\x64\Output\plugins\DirectShowSource.dll  $PortableOutputDir

if ($Architecture -eq 'x64') {
    Write-Step 'Copying VSFilter'
    Copy-ToDirectory $InstallerDepsDir\VSFilter\x64\VSFilter.dll $PortableOutputDir\csri
}
else {
    Write-Step 'Skipping VSFilter (not available for ARM64)'
}

Write-Step 'Copying VC++ runtime'
Copy-ToDirectory (Join-Path $InstallerDepsDir "VC_redist\VC_redist.$Architecture.exe") $PortableOutputDir\Microsoft.CRT

Write-Step 'Copying automation'
Copy-ToDirectory "$InstallerDir\share\aegisub\automation\*"  "$PortableOutputDir\automation\"  -Recurse

Write-Step 'Copying DependencyControl'
Copy-ToDirectory "$InstallerDepsDir\DependencyControl\automation\*"  "$PortableOutputDir\automation\"  -Recurse

Write-Step 'Copying portable config'
Copy-ToDirectory $SourceRoot\packages\win_installer\portable\config.json  $PortableOutputDir

Write-Step 'Creating portable zip'
Remove-Item -LiteralPath $PortableZipPath -Force -ErrorAction SilentlyContinue

# Build the zip in a way that avoids some PowerShell versions emitting backslashes in the entry names.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zipRoot = Split-Path $PortableOutputDir -Leaf
$baseLen = $PortableOutputDir.Length + 1
$zip = [System.IO.Compression.ZipFile]::Open($PortableZipPath, 'Create')
try {
    foreach ($file in Get-ChildItem -LiteralPath $PortableOutputDir -Recurse -File) {
        $entryName = "$zipRoot/" + $file.FullName.Substring($baseLen).Replace('\', '/')
        [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $file.FullName, $entryName, [System.IO.Compression.CompressionLevel]::Optimal)
    }
}
finally {
    $zip.Dispose()
}

Write-Progress -Activity 'Creating portable Aegisub' -Completed
Write-Host "Done: $PortableZipPath"
