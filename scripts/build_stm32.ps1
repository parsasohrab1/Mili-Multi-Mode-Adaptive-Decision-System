# F6 — Cross-compile STM32 firmware (arm-none-eabi-gcc).
param(
    [string]$BuildDir = ""
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $Root
Set-Location $RepoRoot

if (-not $BuildDir) {
    $BuildDir = if ($env:MILI_STM32_BUILD_DIR) { $env:MILI_STM32_BUILD_DIR } else { "build-stm32" }
}

if (-not (Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue)) {
    Write-Error "arm-none-eabi-gcc not found. Install ARM GNU Toolchain."
}

$Toolchain = Join-Path $RepoRoot "platform\stm32\arm-gcc.cmake"

cmake -B $BuildDir -S $RepoRoot `
    -DCMAKE_TOOLCHAIN_FILE=$Toolchain `
    -DMILI_EMBEDDED=ON `
    -DMILI_BUILD_TESTS=OFF `
    -DMILI_BUILD_SIMULATOR=OFF `
    -DMILI_BUILD_EMBEDDED_HAL=ON

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Firmware = Join-Path $BuildDir "platform\stm32\mili_stm32_firmware"
if (Test-Path $Firmware) {
    arm-none-eabi-size $Firmware
}

Write-Host "STM32 firmware built in $BuildDir"
