param(
    [string]$Device = "STM32F407VE",
    [ValidateSet("SWD", "JTAG")]
    [string]$Interface = "SWD",
    [int]$Speed = 4000,
    [string]$Firmware = "build\Industrial_Board.hex"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot

. "$PSScriptRoot\env.ps1"
Set-Location $RepoRoot

$FirmwarePath = Join-Path $RepoRoot $Firmware
if (-not (Test-Path $FirmwarePath)) {
    throw "Firmware file not found: $FirmwarePath. Run scripts/build.ps1 first."
}

$JLink = Get-Command JLink.exe -ErrorAction Stop
$CommandFile = New-TemporaryFile

try {
    @(
        "r",
        "h",
        "loadfile $FirmwarePath",
        "r",
        "g",
        "qc"
    ) | Set-Content -Path $CommandFile -Encoding ASCII

    & $JLink.Source -device $Device -if $Interface -speed $Speed -autoconnect 1 -CommanderScript $CommandFile
    if ($LASTEXITCODE -ne 0) {
        throw "J-Link flashing failed with exit code $LASTEXITCODE."
    }
}
finally {
    Remove-Item $CommandFile -ErrorAction SilentlyContinue
}