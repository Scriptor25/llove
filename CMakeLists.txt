cmake_minimum_required(VERSION 3.31)
project(llove)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(YAML_CPP_BUILD_CONTRIB "" OFF)
option(YAML_CPP_BUILD_TOOLS "" OFF)
option(YAML_BUILD_SHARED_LIBS "" OFF)
option(YAML_CPP_INSTALL "" OFF)
option(YAML_CPP_FORMAT_SOURCE "" OFF)
option(YAML_CPP_DISABLE_UNINSTALL "" OFF)
option(YAML_USE_SYSTEM_GTEST "" OFF)
option(YAML_ENABLE_PIC "" ON)
add_subdirectory(deps/yaml-cpp)

find_package(LLVM 18.1 REQUIRED CONFIG)

execute_process(
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-config --system-libs --libnames core support passes all-targets
        OUTPUT_VARIABLE LLVM_LIBNAMES
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(LLVM_DEFS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
separate_arguments(LLVM_LIBS_LIST NATIVE_COMMAND ${LLVM_LIBNAMES})

message(STATUS "LLVM version ........... ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM config ............ ${LLVM_DIR}")
message(STATUS "LLVM definitions ....... ${LLVM_DEFS_LIST}")
message(STATUS "LLVM libraries ......... ${LLVM_LIBS_LIST}")
message(STATUS "LLVM library directories ${LLVM_LIBRARY_DIRS}")

file(GLOB_RECURSE EXE_SOURCES src/*.cpp)
file(GLOB_RECURSE LIB_SOURCES lib/*.cpp)

add_library(libllove)
set_target_properties(libllove PROPERTIES OUTPUT_NAME "llove")
target_sources(libllove PRIVATE ${LIB_SOURCES})
target_include_directories(libllove PUBLIC include ${LLVM_INCLUDE_DIRS})
target_compile_definitions(libllove PUBLIC ${LLVM_DEFS_LIST})
target_link_directories(libllove PUBLIC ${LLVM_LIBRARY_DIRS})
target_link_libraries(libllove PUBLIC ${LLVM_LIBS_LIST})

set(CONFIG_INPUT ${CMAKE_SOURCE_DIR}/config.yaml)
set(CONFIG_OUTPUT ${CMAKE_BINARY_DIR}/config.h)

add_custom_command(
        OUTPUT ${CONFIG_OUTPUT}
        COMMAND xxd -i -n config_data ${CONFIG_INPUT} ${CONFIG_OUTPUT}
        DEPENDS ${CONFIG_INPUT}
        COMMENT "embedding config.yaml with xxd"
)
add_custom_target(generate_config DEPENDS ${CONFIG_OUTPUT})

add_executable(llove)
add_dependencies(llove generate_config)
set_target_properties(llove PROPERTIES OUTPUT_NAME "llove")
target_sources(llove PRIVATE ${EXE_SOURCES})
target_include_directories(llove PRIVATE ${CMAKE_BINARY_DIR} include)
target_link_libraries(llove PRIVATE libllove yaml-cpp::yaml-cpp)
