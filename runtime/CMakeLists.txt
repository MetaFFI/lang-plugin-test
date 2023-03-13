add_boost(filesystem)

hunter_add_package(Boost COMPONENTS filesystem)
add_boost(filesystem)

set(target_name "xllr.test")

# build xllr.test
file(GLOB ${target_name}_sources
		${CMAKE_CURRENT_LIST_DIR}/*.cpp
		${CMAKE_CURRENT_LIST_DIR}/*.h
		${METAFFI_SDK}/runtime/cdt_capi_loader.c
		${METAFFI_SDK}/runtime/cdts_wrapper.cpp)
add_library(${target_name} SHARED ${${target_name}_sources})
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${METAFFI_SDK})
set_target_properties(${target_name} PROPERTIES PREFIX "" POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${target_name} PRIVATE Boost::filesystem)

# unitest
add_executable(xllr_test_plugin ${CMAKE_CURRENT_LIST_DIR}/test_plugin_test.cpp)
target_link_libraries(xllr_test_plugin PRIVATE ${CMAKE_DL_LIBS} Catch2::Catch2)
add_test(NAME "Test XLLR with Test plugin" COMMAND xllr_test_plugin)


install_target(${target_name} .)