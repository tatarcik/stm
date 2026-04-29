param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

. "$PSScriptRoot\env.ps1"
Set-Location $RepoRoot

if ($Clean) {
    make clean
}

make
