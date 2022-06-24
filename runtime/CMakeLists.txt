find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library

include(CTest)
enable_testing()

set(target_name "xllr.test")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.h ${METAFFI_SDK}/runtime/*.c ${METAFFI_SDK}/utils/*.cpp ${METAFFI_SDK}/runtime/*.cpp)
# build xllr.test
add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} $ENV{METAFFI_HOME} ${METAFFI_SDK}/utils/*.h ${METAFFI_SDK}/runtime/*.h)

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename "xllr.test.[ext]"
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

if(NOT WIN32)
	target_link_libraries(${target_name} Boost::filesystem pthread dl)
else()
	target_link_libraries(${target_name} Boost::filesystem Boost::dynamic_linking)
endif()

# Copy to metaffi directory
#add_custom_command(TARGET ${target_name} POST_BUILD
#		COMMAND "${CMAKE_COMMAND}" -E copy
#		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
#		"$ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
#		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_executable(xllr_test ${CMAKE_CURRENT_LIST_DIR}/test_plugin_test.c)
if(NOT WIN32)
	target_link_libraries(xllr_test dl)
endif()

add_test(NAME xllr_test COMMAND xllr_test)
