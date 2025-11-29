#!/bin/bash

# Build script for Trade Matching Engine
# This script handles the complete build process including dependency checks

set -e  # Exit on any error

echo "=================================================="
echo "Trade Matching Engine Build Script"
echo "=================================================="

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check prerequisites
echo "Checking prerequisites..."

if ! command_exists cmake; then
    echo "Error: CMake is not installed"
    echo "Please install CMake 3.16 or later"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "On macOS: brew install cmake"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "On Ubuntu/Debian: sudo apt install cmake"
    fi
    exit 1
fi

if ! command_exists g++ && ! command_exists clang++; then
    echo "Error: No C++ compiler found"
    echo "Please install a C++17 compatible compiler"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "On macOS: xcode-select --install"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "On Ubuntu/Debian: sudo apt install build-essential"
    fi
    exit 1
fi

echo "✓ Prerequisites check passed"

# Parse command line arguments
BUILD_TYPE="Release"
RUN_TESTS=false
INSTALL=false
CLEAN=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug     Build in debug mode (default: release)"
            echo "  --test      Run tests after building"
            echo "  --install   Install the built binaries"
            echo "  --clean     Clean build directory first"
            echo "  --verbose   Enable verbose build output"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Create build directory
BUILD_DIR="build"

if [[ "$CLEAN" == true ]] && [[ -d "$BUILD_DIR" ]]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure
echo "Configuring build (${BUILD_TYPE})..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

cmake .. $CMAKE_ARGS

# Determine number of cores for parallel build
if command_exists nproc; then
    CORES=$(nproc)
elif command_exists sysctl; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

# Build
echo "Building with $CORES cores..."
if [[ "$VERBOSE" == true ]]; then
    make -j$CORES VERBOSE=1
else
    make -j$CORES
fi

echo "✓ Build completed successfully"

# Run tests if requested
if [[ "$RUN_TESTS" == true ]]; then
    echo "Running tests..."
    
    if [[ -f "TradeMatchEngine_tests" ]]; then
        echo "Running unit tests..."
        ./TradeMatchEngine_tests
        echo "✓ Unit tests passed"
    else
        echo "Warning: Tests not built (Google Test not found)"
    fi
    
    if [[ -f "performance_test" ]]; then
        echo "Running performance tests..."
        ./performance_test
        echo "✓ Performance tests completed"
    fi
fi

# Install if requested
if [[ "$INSTALL" == true ]]; then
    echo "Installing..."
    if [[ "$EUID" -ne 0 ]]; then
        echo "Note: May require sudo for system installation"
    fi
    make install
    echo "✓ Installation completed"
fi

# Summary
echo ""
echo "=================================================="
echo "Build Summary"
echo "=================================================="
echo "Build type: $BUILD_TYPE"
echo "Build directory: $(pwd)"

if [[ -f "TradeMatchEngine_main" ]]; then
    echo "✓ Main executable: TradeMatchEngine_main"
fi

if [[ -f "libTradeMatchEngine.a" ]]; then
    echo "✓ Static library: libTradeMatchEngine.a"
fi

if [[ -f "TradeMatchEngine_tests" ]]; then
    echo "✓ Test executable: TradeMatchEngine_tests"
fi

if [[ -f "performance_test" ]]; then
    echo "✓ Performance test: performance_test"
fi

echo ""
echo "To run the demo:"
echo "  ./TradeMatchEngine_main"
echo ""
echo "To run tests:"
echo "  ./TradeMatchEngine_tests"
echo ""
echo "To run performance benchmarks:"
echo "  ./performance_test"
echo "  ./TradeMatchEngine_main --benchmark"
echo ""
echo "Build completed successfully!"