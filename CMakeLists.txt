cmake_minimum_required(VERSION 3.31)
project(LLove)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM 18.1 REQUIRED CONFIG)

execute_process(
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-config --system-libs --libnames core support passes all-targets
        OUTPUT_VARIABLE LLVM_LIBNAMES
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(LLVM_LIBNAMES_LIST NATIVE_COMMAND ${LLVM_LIBNAMES})
separate_arguments(LLVM_DEFS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

message(STATUS "LLVM version ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM config ${LLVM_DIR}")
message(STATUS "LLVM definitions ${LLVM_DEFS_LIST}")
message(STATUS "LLVM libraries ${LLVM_LIBNAMES_LIST}")
message(STATUS "LLVM library directories ${LLVM_LIBRARY_DIRS}")

file(GLOB_RECURSE LLOVE_SOURCE src/*.cpp)
add_executable(llove ${LLOVE_SOURCE})
target_include_directories(llove PRIVATE include ${LLVM_INCLUDE_DIRS})
target_compile_definitions(llove PRIVATE ${LLVM_DEFS_LIST})
target_link_directories(llove PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(llove PRIVATE ${LLVM_LIBNAMES_LIST})
