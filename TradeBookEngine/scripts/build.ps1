# PowerShell build script for Trade Booking Engine
# Run this with: .\scripts\build.ps1

Write-Host "=== Trade Booking Engine Build Script ===" -ForegroundColor Green
Write-Host "New improved project structure with lib/, apps/, include/" -ForegroundColor Cyan

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Name "build" | Out-Null
}

Set-Location "build"

# Check for available build tools
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
$clang = Get-Command clang++ -ErrorAction SilentlyContinue
$msbuild = Get-Command msbuild -ErrorAction SilentlyContinue

if ($cmake) {
    Write-Host "Using CMake..." -ForegroundColor Yellow
    try {
        & cmake .. 2>&1 | Tee-Object -Variable cmakeOutput
        if ($LASTEXITCODE -eq 0) {
            & cmake --build . 2>&1 | Tee-Object -Variable buildOutput
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ CMake build successful!" -ForegroundColor Green
                $success = $true
            } else {
                Write-Host "✗ CMake build failed" -ForegroundColor Red
                $success = $false
            }
        } else {
            Write-Host "✗ CMake configuration failed" -ForegroundColor Red
            $success = $false
        }
    } catch {
        Write-Host "✗ CMake execution failed: $_" -ForegroundColor Red
        $success = $false
    }
} elseif ($gpp) {
    Write-Host "Using GCC..." -ForegroundColor Yellow
    try {
        # Compile core sources
        $coreSources = Get-ChildItem "../lib/core/src/*.cpp" | ForEach-Object { $_.FullName }
        & g++ -c -std=c++17 -I"../include" @coreSources 2>&1 | Tee-Object -Variable compileOutput
        
        if ($LASTEXITCODE -eq 0) {
            # Create static library
            & ar rcs libTradeBookEngineCore.a *.o 2>&1 | Out-Null
            
            # Compile main application
            & g++ -std=c++17 -I"../include" "../apps/console/main.cpp" "libTradeBookEngineCore.a" -o "tradebook_console.exe" 2>&1 | Tee-Object -Variable linkOutput
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ GCC build successful!" -ForegroundColor Green
                $success = $true
            } else {
                Write-Host "✗ GCC linking failed" -ForegroundColor Red
                $success = $false
            }
        } else {
            Write-Host "✗ GCC compilation failed" -ForegroundColor Red
            $success = $false
        }
    } catch {
        Write-Host "✗ GCC execution failed: $_" -ForegroundColor Red
        $success = $false
    }
} elseif ($clang) {
    Write-Host "Using Clang..." -ForegroundColor Yellow
    # Similar logic as GCC but with clang++
    $success = $false
    Write-Host "Clang build not implemented in this script yet" -ForegroundColor Yellow
} else {
    Write-Host "No suitable compiler found!" -ForegroundColor Red
    Write-Host "Please install one of:" -ForegroundColor Yellow
    Write-Host "  - CMake: https://cmake.org/download/" -ForegroundColor Cyan
    Write-Host "  - MinGW-w64: https://www.mingw-w64.org/" -ForegroundColor Cyan
    Write-Host "  - Visual Studio Build Tools" -ForegroundColor Cyan
    $success = $false
}

# Check results
if ($success) {
    if (Test-Path "tradebook_console.exe") {
        Write-Host "`n✓ Executable created: build/tradebook_console.exe" -ForegroundColor Green
        Write-Host "Run with: .\tradebook_console.exe" -ForegroundColor Cyan
    } elseif (Test-Path "bin/tradebook_console.exe") {
        Write-Host "`n✓ Executable created: build/bin/tradebook_console.exe" -ForegroundColor Green
        Write-Host "Run with: .\bin\tradebook_console.exe" -ForegroundColor Cyan
    } else {
        Write-Host "`n? Build completed but executable not found" -ForegroundColor Yellow
    }
} else {
    Write-Host "`n✗ Build failed. Check the output above for errors." -ForegroundColor Red
}

# Return to original directory
Set-Location ".."

Write-Host "`n=== Build Script Complete ===" -ForegroundColor Green