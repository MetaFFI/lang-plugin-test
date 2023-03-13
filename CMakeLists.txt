cmake_minimum_required(VERSION 3.16.3)

if("$ENV{METAFFI_HOME}" STREQUAL "")
	message(FATAL_ERROR "METAFFI_HOME is missing - MetaFFI must be installed")
endif()

# policies
cmake_policy(SET CMP0110 NEW) # Allow arbitrary names in CTest names

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DEBUG) # add -DEBUG to "Debug" builds
	set(CMAKE_DEBUG_POSTFIX "")
endif()

# Add SDK
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/plugin-sdk/)
set(METAFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk)
set(CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk/cmake)

# Load hunter package manager
load_hunter_pm(${CMAKE_SCRIPTS_DIR}) # load package manager

# include scripts required for project
include(${CMAKE_SCRIPTS_DIR}/InstallUtils.cmake)

#-----------------------------------------------------------------------------------------------

project(lang-plugin-test VERSION 0.0.1)

# Add required packages
hunter_add_package(Catch)
add_ctest_and_catch2_packages()

# set RPATH
set(CMAKE_MACOSX_RPATH 1) # tell MacOS RPATH is in use
list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN")
list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/lib")

# add targets
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/runtime/)

# cmake install (not cpack)
set(CMAKE_INSTALL_PREFIX [[$ENV{METAFFI_HOME}]])
set(CMAKE_SKIP_INSTALL_RPATH ON)

#-----------------------------------------------------------------------------------------------

# build installer
set(CPACK_PACKAGE_NAME "MetaFFITestLanguagePlugin")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME} CACHE STRING "MetaFFI")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "metaffi")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/MetaFFI/")

install(CODE [[
set(CPACK_PACKAGE_INSTALL_DIRECTORY "$ENV{METAFFI_HOME}") # IMPORTANT: INSTALL TO METAFFI_HOME
]])

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
	set(CPACK_GENERATOR "DEB")
	set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
	set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/metaffi/")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tsvi Cherny-Shahar")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "metaffi")

elseif(WIN32)

	set(CPACK_GENERATOR "WIX")
	set(CPACK_WIX_PRODUCT_GUID "6b56c8c0-49c7-4794-8def-0b706a08c55f")
	set(CPACK_WIX_UPGRADE_GUID "fe787db7-885d-4d9d-89d8-5208fb4a8fda")
	set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/MetaFFI/")
	set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/installer/WixExtraCommands.xml")

endif()

set(CPACK_RESOURCE_FILE_LICENSE    ${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README     ${CMAKE_CURRENT_LIST_DIR}/README.md)

include(CPack)