param (
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Store the root directory where the script is executed
$ProjectRoot = Get-Location
$CppDir = Join-Path -Path $ProjectRoot -ChildPath "src/cpp"

Write-Host " Building RF Simulation Engine & Tests   " -ForegroundColor Cyan

# Verify C++ directory exists and navigate to it
if (-not (Test-Path $CppDir)) {
    Write-Host "`n[!] Could not find '$CppDir'. Please ensure you are running this from the project root." -ForegroundColor Red
    exit 1
}

Set-Location -Path $CppDir

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

# Compile the C++ project in Release mode
if ($Clean) {
    Write-Host "`nCompiling C++ Code (Clean Build)..." -ForegroundColor Yellow
    cmake --build . --config Release --clean-first
} else {
    Write-Host "`nCompiling C++ Code (Incremental)..." -ForegroundColor Yellow
    cmake --build . --config Release
}

# Run the unit tests
Write-Host "`nExecuting Unit Tests..." -ForegroundColor Yellow
ctest -C Release --output-on-failure

# If tests fail, halt the script before building the Node addon
if ($LASTEXITCODE -ne 0) {
    Write-Host "`n[!] Unit tests failed. Halting Node.js build." -ForegroundColor Red
    # Navigate back to the project root for convenience before exiting
    Set-Location -Path $ProjectRoot
    exit $LASTEXITCODE
}

# Return to project root to build the Node Addon
Set-Location -Path $ProjectRoot

Write-Host "`t`tBuild and Test Complete!" -ForegroundColor Green