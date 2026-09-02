param(
    [ValidateSet('x64', 'arm64')]
    [string] $Architecture = 'x64',
    [string] $BuildDirectory = '',
    [switch] $Installer
)

$ErrorActionPreference = 'Stop'

if (-not $BuildDirectory) {
    $BuildDirectory = "build-$($Architecture.ToLowerInvariant())"
}

if (-not $env:VS) {
    throw 'Visual Studio path is not set. Run from a Visual Studio developer prompt or set VS.'
}

$vsDevShell = Join-Path $env:VS 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll'
if (-not (Test-Path $vsDevShell)) {
    throw 'Visual Studio DevShell was not found. Run from a Visual Studio developer prompt or set VS.'
}

Import-Module $vsDevShell
Enter-VsDevShell -VsInstallPath $env:VS -SkipAutomaticLocation `
    -DevCmdArguments "-arch=$($Architecture.ToLowerInvariant()) -host_arch=x64"

meson setup $BuildDirectory --reconfigure -Ddefault_library=static
meson compile -C $BuildDirectory

if ($Installer) {
    if ($Architecture -ne 'x64') {
        throw 'The current installer payload is x64-only. Build an ARM64 binary without -Installer.'
    }
    foreach ($tool in @('iscc.exe', '7z.exe', 'moonc.exe', 'msgfmt.exe')) {
        if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
            throw "$tool is required for installer generation and must be on PATH."
        }
    }
    meson compile -C $BuildDirectory win-installer
    Write-Host "Installer output: $((Resolve-Path $BuildDirectory).Path)\Aegisub-*.exe"
}
