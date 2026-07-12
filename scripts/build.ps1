# F1 — CMake build wrapper (avoids Windows MAX_PATH with short build dir).
param(
    [string]$BuildDir = "",
    [string]$Config = "Release",
    [switch]$Coverage,
    [switch]$Test,
    [switch]$Simulator
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $Root
Set-Location $RepoRoot

if (-not $BuildDir) {
    if ($env:MILI_BUILD_DIR) {
        $BuildDir = $env:MILI_BUILD_DIR
    } else {
        # Short path avoids MAX_PATH when clone path is deep (see docs/BUILD.md).
        $BuildDir = "C:\mili-build"
    }
}

$CmakeArgs = @("-B", $BuildDir, "-S", $RepoRoot)
if ($Coverage) {
    $CmakeArgs += @("-DCMAKE_BUILD_TYPE=Debug", "-DMILI_ENABLE_COVERAGE=ON")
} else {
    $CmakeArgs += @("-DCMAKE_BUILD_TYPE=$Config")
}

Write-Host "Configure: cmake $($CmakeArgs -join ' ')"
cmake @CmakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build: cmake --build $BuildDir --config $Config"
cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Test) {
    Write-Host "Test: ctest --test-dir $BuildDir -C $Config --output-on-failure"
    ctest --test-dir $BuildDir -C $Config --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($Simulator) {
    $Sim = Join-Path $BuildDir "Release\mili_simulator.exe"
    if (-not (Test-Path $Sim)) {
        $Sim = Join-Path $BuildDir "mili_simulator.exe"
    }
    Write-Host "Run: $Sim"
    & $Sim
}

Write-Host "Done. Build dir: $BuildDir"
