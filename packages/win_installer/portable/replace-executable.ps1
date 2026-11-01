#!/usr/bin/env powershell

# Replaces aegisub.exe in a portable zip created by create-portable.ps1, e.g.
# with a signed copy, leaving all other entries untouched.

param (
    [Parameter(Position = 0, Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$PortableZip,
    [Parameter(Position = 1, Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Executable
)

$ErrorActionPreference = 'Stop'

$PortableZip = (Resolve-Path -LiteralPath $PortableZip).Path
$Executable = (Resolve-Path -LiteralPath $Executable).Path
$EntryName = 'aegisub-portable/aegisub.exe'

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [System.IO.Compression.ZipFile]::Open($PortableZip, 'Update')
try {
    $matching = @($zip.Entries | Where-Object { $_.FullName.Replace('\', '/') -ieq $EntryName })
    if ($matching.Count -ne 1) {
        throw "Expected one $EntryName in $PortableZip, found $($matching.Count)"
    }
    $matching[0].Delete()
    [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $Executable, $EntryName, [System.IO.Compression.CompressionLevel]::Optimal)
}
finally {
    $zip.Dispose()
}

# Check that the zip now contains exactly the given executable
$zip = [System.IO.Compression.ZipFile]::OpenRead($PortableZip)
try {
    $stream = $zip.GetEntry($EntryName).Open()
    try {
        $zipped = (Get-FileHash -InputStream $stream -Algorithm SHA256).Hash
    }
    finally {
        $stream.Dispose()
    }
}
finally {
    $zip.Dispose()
}
if ($zipped -ne (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash) {
    throw "$EntryName in $PortableZip does not match $Executable"
}

Write-Host "Replaced $EntryName in $PortableZip"
