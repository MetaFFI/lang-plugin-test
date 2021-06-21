find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library

include(CTest)
enable_testing()

set(target_name "xllr.test")

file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.h ${OPENFFI_SDK}/runtime/*.c ${OPENFFI_SDK}/utils/*.cpp ${OPENFFI_SDK}/runtime/*.cpp)
# build xllr.test
add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} $ENV{OPENFFI_HOME} ${OPENFFI_SDK}/utils/*.h ${OPENFFI_SDK}/runtime/*.h)

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename "xllr.test.[ext]"
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

target_link_libraries(${target_name} Boost::filesystem)

# Copy to openffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{OPENFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{OPENFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_executable(xllr_test ${CMAKE_CURRENT_LIST_DIR}/test_plugin_test.c)
target_link_libraries(xllr_test dl)