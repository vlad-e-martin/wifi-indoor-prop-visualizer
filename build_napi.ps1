param (
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Build the Node.js N-API Addon
Write-Host "`nCompiling Node.js N-API Binding (cmake-js)..." -ForegroundColor Yellow
if ($Clean) {
    # cmake-js will wipe its own cache and rebuild
    cmake-js compile --clean
} else {
    cmake-js compile
}

Write-Host "`t`tBuild Node Addon Complete!" -ForegroundColor Green