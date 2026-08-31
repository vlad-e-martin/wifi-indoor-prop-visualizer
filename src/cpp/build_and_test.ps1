param (
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Check if the user requested a clean
if ($Clean) {
    if (Test-Path "build") {
        Write-Host "`nClean flag detected. Removing old build directory..." -ForegroundColor Yellow
        Remove-Item -Path "build" -Recurse -Force
    } else {
        Write-Host "`nClean flag detected, but no build directory exists. Proceeding..." -ForegroundColor Yellow
    }
}

# Create build directory if it is not available yet
if (-not (Test-Path "build")) {
    Write-Host "`nCreating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
} else {
    Write-Host "`nBuild directory already exists." -ForegroundColor Yellow
}
# Move into build directory while building
Set-Location -Path "build"

Write-Host "`nConfiguring CMake..." -ForegroundColor Yellow
cmake ..

if ($Clean) {
    Write-Host "`nCompiling C++ Code (Clean Build)..." -ForegroundColor Yellow
    cmake --build . --config Release --clean-first
} else {
    Write-Host "`nCompiling C++ Code (Incremental)..." -ForegroundColor Yellow
    cmake --build . --config Release
}

Write-Host "`nExecuting Unit Tests..." -ForegroundColor Yellow
ctest -C Release --output-on-failure

# Return to root directory after code is built & tests ran
Set-Location -Path ".."