# Root CMakeLists for canonical service layout
cmake_minimum_required(VERSION 3.10)
project(FusionMatchTrade_CanonicalServices)

enable_testing()

add_subdirectory(tradebook)
add_subdirectory(matchengine)
add_subdirectory(forex)
