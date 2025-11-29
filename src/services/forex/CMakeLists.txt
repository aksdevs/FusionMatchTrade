cmake_minimum_required(VERSION 3.10)
project(forex_service)

# Use service-local copies of sources, headers and proto
file(GLOB_RECURSE FOREX_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
# Remove the server implementation from the initial glob; we'll only compile it
# when protobuf/grpc generation succeeded and the generated grpc header exists.
list(REMOVE_ITEM FOREX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/forex_service.cpp")
file(GLOB_RECURSE FOREX_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.h"
)

find_package(Protobuf QUIET)

# Try to locate grpc plugin and headers; avoid using find_package(gRPC) directly because
# system cmake config for gRPC can require the plugin to exist and cause hard errors.
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
find_path(GRPC_INCLUDE_DIR grpcpp/grpcpp.h)

if (Protobuf_FOUND)
        message(STATUS "Protobuf found: ${Protobuf_INCLUDE_DIRS}")
        include_directories(${Protobuf_INCLUDE_DIRS})
        set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/forex.proto")
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})
        list(APPEND FOREX_SRCS ${PROTO_SRCS})
        list(APPEND FOREX_HDRS ${PROTO_HDRS})
        # If gRPC is available, try to generate grpc sources as well
        # Attempt to generate grpc sources if both the grpc plugin and grpc headers are available
        if(GRPC_CPP_PLUGIN AND GRPC_INCLUDE_DIR)
            set(GRPC_PLUGIN ${GRPC_CPP_PLUGIN})
            get_filename_component(PROTO_ABS ${PROTO_FILE} ABSOLUTE)
            execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --grpc_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_PLUGIN} -I ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_ABS} RESULT_VARIABLE _GRPC_GEN_RET)
            if(_GRPC_GEN_RET EQUAL 0)
                list(APPEND FOREX_SRCS ${CMAKE_CURRENT_BINARY_DIR}/forex.grpc.pb.cc)
                list(APPEND FOREX_HDRS ${CMAKE_CURRENT_BINARY_DIR}/forex.grpc.pb.h)
            endif()
        endif()
else()
        message(STATUS "Protobuf not found; proto tests will be skipped and generated sources won't be built")
endif()

## Only compile the gRPC-based server source when gRPC is present
set(FOREX_COMPILE_SRCS ${FOREX_SRCS})
# Only compile the gRPC-based server source when grpc headers are present
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/forex.grpc.pb.h")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/forex_service.cpp")
        list(APPEND FOREX_COMPILE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/forex_service.cpp)
    endif()
else()
    message(STATUS "forex.grpc.pb.h not present; skipping building forex gRPC server implementation")
endif()

add_library(forex STATIC ${FOREX_COMPILE_SRCS} ${FOREX_HDRS})
target_include_directories(forex PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(forex PROPERTIES
    OUTPUT_NAME "forex"
)

if (Protobuf_FOUND)
        enable_testing()
        # Prefer repo-level test under <repo-root>/tests/services/forex
        set(FOREX_TEST_SRC "")
        if(EXISTS "${CMAKE_SOURCE_DIR}/../../tests/services/forex/test_forex_proto.cpp")
            set(FOREX_TEST_SRC "${CMAKE_SOURCE_DIR}/../../tests/services/forex/test_forex_proto.cpp")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_forex_proto.cpp")
            set(FOREX_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_forex_proto.cpp")
        endif()

        if(FOREX_TEST_SRC)
            add_executable(test_forex_proto ${FOREX_TEST_SRC} ${PROTO_SRCS})
            target_include_directories(test_forex_proto PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
            target_link_libraries(test_forex_proto PRIVATE ${Protobuf_LIBRARIES})
            add_test(NAME test_forex_proto COMMAND test_forex_proto)
        else()
            message(STATUS "Protobuf found but forex proto test source not present; skipping test")
        endif()
else()
        message(STATUS "Skipping forex proto test because Protobuf not found")
endif()
