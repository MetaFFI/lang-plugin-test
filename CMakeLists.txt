cmake_minimum_required(VERSION 3.17.3)
project(openffi.lang.test VERSION 0.0.1)

include(CTest)
enable_testing()

set(OPENFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/../plugin-sdk)
include(${OPENFFI_SDK}/CMakeLists.txt)

include(runtime/CMakeLists.txt)