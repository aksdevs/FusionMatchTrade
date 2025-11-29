cmake_minimum_required(VERSION 3.20)
## Prefer Clang if available and not already set
if(NOT CMAKE_CXX_COMPILER)
    find_program(CLANGPP clang++)
    if(CLANGPP)
        set(CMAKE_CXX_COMPILER ${CLANGPP} CACHE STRING "C++ compiler" FORCE)
    endif()
endif()
project(TradeBookEngine 
    VERSION 1.0.0
    DESCRIPTION "A C++ trade booking engine with modular architecture"
    LANGUAGES CXX
)

# Options
option(TRADEBOOK_ENABLE_WERROR "Treat warnings as errors" OFF)


# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    if(TRADEBOOK_ENABLE_WERROR)
        add_compile_options(/WX)
    endif()
else()
    # Common warnings for GCC/Clang
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wuninitialized -Wunused -Wsign-conversion)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wimplicit-fallthrough -Wshorten-64-to-32)
    endif()
    if(TRADEBOOK_ENABLE_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Enable testing
enable_testing()

# Add subprojects
add_subdirectory(lib/core)
add_subdirectory(apps/console)

# Optional: Add tests if available
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Configuration Summary ===")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "  Binary Dir: ${CMAKE_BINARY_DIR}")
message(STATUS "==============================")
message(STATUS "")