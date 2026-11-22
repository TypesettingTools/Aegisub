#!/usr/bin/env powershell

# Replaces executables in a portable zip created by create-portable.ps1, e.g.
# with signed copies, leaving all other entries untouched.
#
#   replace-executable.ps1 Aegisub-...-portable.zip signed\aegisub.exe signed\aegisub-cli.exe

param (
    [Parameter(Position = 0, Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$PortableZip,
    [Parameter(Position = 1, Mandatory = $true, ValueFromRemainingArguments = $true)]
    [ValidateNotNullOrEmpty()]
    [string[]]$Executables
)

$ErrorActionPreference = 'Stop'

$PortableZip = (Resolve-Path -LiteralPath $PortableZip).Path
$Executables = @($Executables | ForEach-Object { (Resolve-Path -LiteralPath $_).Path })

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [System.IO.Compression.ZipFile]::Open($PortableZip, 'Update')
try {
    foreach ($Executable in $Executables) {
        $EntryName = 'aegisub-portable/' + (Split-Path -Leaf $Executable)
        $matching = @($zip.Entries | Where-Object { $_.FullName.Replace('\', '/') -ieq $EntryName })
        if ($matching.Count -ne 1) {
            throw "Expected one $EntryName in $PortableZip, found $($matching.Count)"
        }
        $matching[0].Delete()
        [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $Executable, $EntryName, [System.IO.Compression.CompressionLevel]::Optimal)
    }
}
finally {
    $zip.Dispose()
}

# Check that the zip now contains exactly the given executables
$zip = [System.IO.Compression.ZipFile]::OpenRead($PortableZip)
try {
    foreach ($Executable in $Executables) {
        $EntryName = 'aegisub-portable/' + (Split-Path -Leaf $Executable)
        $stream = $zip.GetEntry($EntryName).Open()
        try {
            $zipped = (Get-FileHash -InputStream $stream -Algorithm SHA256).Hash
        }
        finally {
            $stream.Dispose()
        }
        if ($zipped -ne (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash) {
            throw "$EntryName in $PortableZip does not match $Executable"
        }
        Write-Host "Replaced $EntryName in $PortableZip"
    }
}
finally {
    $zip.Dispose()
}
