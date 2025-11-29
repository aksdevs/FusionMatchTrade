cmake_minimum_required(VERSION 3.20)

# Core library configuration
project(TradeBookEngineCore LANGUAGES CXX)

# Collect all source files
file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create the core library
add_library(TradeBookEngineCore STATIC ${CORE_SOURCES})

# Set target properties
target_compile_features(TradeBookEngineCore PUBLIC cxx_std_17)

# Include directories - use the root include directory
target_include_directories(TradeBookEngineCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set output directory
set_target_properties(TradeBookEngineCore PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    OUTPUT_NAME "tradebookengine_core"
)

# Installation rules
install(TARGETS TradeBookEngineCore
    EXPORT TradeBookEngineTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

message(STATUS "Core library configured with ${CORE_SOURCES}")
message(STATUS "Include directory: ${CMAKE_SOURCE_DIR}/include")