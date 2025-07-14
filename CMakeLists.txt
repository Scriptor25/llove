cmake_minimum_required(VERSION 3.31)
project(llove)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_library(lib)
set_target_properties(lib PROPERTIES OUTPUT_NAME "llove")
target_sources(lib PRIVATE ${LIB_SOURCES})
target_include_directories(lib PUBLIC include ${LLVM_INCLUDE_DIRS})
target_compile_definitions(lib PUBLIC ${LLVM_DEFS_LIST})
target_link_directories(lib PUBLIC ${LLVM_LIBRARY_DIRS})
target_link_libraries(lib PUBLIC ${LLVM_LIBS_LIST})

add_executable(exe)
set_target_properties(exe PROPERTIES OUTPUT_NAME "llove")
target_sources(exe PRIVATE ${EXE_SOURCES})
target_include_directories(exe PRIVATE include)
target_link_libraries(exe PRIVATE lib)
