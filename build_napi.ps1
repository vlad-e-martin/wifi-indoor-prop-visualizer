param (
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Build the Node.js N-API Addon
Write-Host "`nCompiling Node.js N-API Binding (cmake-js)..." -ForegroundColor Yellow
if ($Clean) {
    # cmake-js will wipe its own cache and rebuild
    # TODO: Temporary solution to point to manual absolute path, will only work on Windows systems with MSYS
    cmake-js compile --clean -G "Ninja" --CD CMAKE_C_COMPILER="C:/msys64/ucrt64/bin/cc.exe" --CD CMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/c++.exe"
} else {
    cmake-js compile -G "Ninja" --CD CMAKE_C_COMPILER="C:/msys64/ucrt64/bin/cc.exe" --CD CMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/c++.exe"
}

Write-Host "`t`tBuild Node Addon Complete!" -ForegroundColor Green