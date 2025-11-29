@echo off
REM Build script for Trade Booking Engine (Windows)
REM This script attempts to build the project using available tools

echo Building Trade Booking Engine...

REM Check if CMake is available
where cmake >nul 2>&1
if %errorlevel% == 0 (
    echo Using CMake build...
    if not exist "build" mkdir build
    cd build
    cmake ..
    cmake --build .
    goto :end
)

REM Check if Visual Studio is available
where msbuild >nul 2>&1
if %errorlevel% == 0 (
    echo Using MSBuild...
    REM Create a simple VS project structure would go here
    goto :manual_build
)

REM Check if MinGW/GCC is available
where g++ >nul 2>&1
if %errorlevel% == 0 (
    echo Using GCC...
    goto :gcc_build
)

REM Check if Clang is available
where clang++ >nul 2>&1
if %errorlevel% == 0 (
    echo Using Clang...
    goto :clang_build
)

:manual_build
echo.
echo No suitable build system found automatically.
echo Please install one of the following:
echo   - CMake (recommended): https://cmake.org/download/
echo   - Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/
echo   - MinGW-w64: https://www.mingw-w64.org/downloads/
echo   - LLVM/Clang: https://releases.llvm.org/download.html
echo.
echo Manual compilation instructions:
echo 1. Compile all .cpp files in lib/core/src/
echo 2. Create a static library from the object files
echo 3. Compile apps/console/main.cpp and link with the static library
echo.
goto :end

:gcc_build
echo Compiling with GCC...
if not exist "build" mkdir build
cd build

REM Compile core library
g++ -c -std=c++17 -I../include ^
    ../lib/core/src/*.cpp
ar rcs libTradeBookEngineCore.a *.o

REM Compile console application
g++ -std=c++17 -I../include ^
    ../apps/console/main.cpp ^
    libTradeBookEngineCore.a -o tradebook_console.exe

echo Build complete: build/tradebook_console.exe
goto :end

:clang_build
echo Compiling with Clang...
if not exist "build" mkdir build
cd build

REM Compile core library
clang++ -c -std=c++17 -I../include ^
    ../lib/core/src/*.cpp
lib /out:TradeBookEngineCore.lib *.obj 2>nul || ar rcs libTradeBookEngineCore.a *.o

REM Compile console application
clang++ -std=c++17 -I../include ^
    ../apps/console/main.cpp ^
    TradeBookEngineCore.lib -o tradebook_console.exe 2>nul || ^
clang++ -std=c++17 -I../include ^
    ../apps/console/main.cpp ^
    libTradeBookEngineCore.a -o tradebook_console.exe

echo Build complete: build/tradebook_console.exe
goto :end

:end
echo.
echo Build process completed.
if exist "build/tradebook_console.exe" (
    echo Run: build/tradebook_console.exe
) else if exist "build/bin/tradebook_console.exe" (
    echo Run: build/bin/tradebook_console.exe
) else (
    echo Check build output for any errors.
)