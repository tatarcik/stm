$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

. "$PSScriptRoot\env.ps1"
Set-Location $RepoRoot

make flash
