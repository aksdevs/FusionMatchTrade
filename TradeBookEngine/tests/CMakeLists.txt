cmake_minimum_required(VERSION 3.20)
project(TradeBookEngineTests LANGUAGES CXX)

# Enable testing
enable_testing()

set(TEST_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/test_trade_service.cpp
)

add_executable(trade_service_tests ${TEST_SOURCES})
target_compile_features(trade_service_tests PRIVATE cxx_std_17)

target_include_directories(trade_service_tests PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(trade_service_tests PRIVATE TradeBookEngineCore)

add_test(NAME trade_service_tests COMMAND trade_service_tests)

message(STATUS "Configured unit tests: trade_service_tests")