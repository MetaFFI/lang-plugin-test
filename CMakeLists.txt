cmake_minimum_required(VERSION 3.17.3)
project(metaffi.lang.test VERSION 0.0.1)

include(CTest)
enable_testing()

set(METAFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/../plugin-sdk)
include(${METAFFI_SDK}/CMakeLists.txt)

include(runtime/CMakeLists.txt)