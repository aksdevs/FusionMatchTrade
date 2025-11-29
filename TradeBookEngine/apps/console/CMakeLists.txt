cmake_minimum_required(VERSION 3.20)

# Console application configuration
project(TradeBookEngineConsole LANGUAGES CXX)

# Create the executable
add_executable(TradeBookEngineConsole main.cpp)

# Link with core library
target_link_libraries(TradeBookEngineConsole PRIVATE TradeBookEngineCore)

# Set target properties
target_compile_features(TradeBookEngineConsole PRIVATE cxx_std_17)

# Include directories - use root include directory
target_include_directories(TradeBookEngineConsole PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Set output directory and name
set_target_properties(TradeBookEngineConsole PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "tradebook_console"
)

# Installation
install(TARGETS TradeBookEngineConsole
    RUNTIME DESTINATION bin
)

message(STATUS "Console application configured")