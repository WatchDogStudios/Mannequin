cmake_minimum_required(VERSION 3.20)

# Detect project name from folder structure
include("BuildSystem/CMake/CMakeUtils/nsUtilsDetect.cmake")
ns_detect_project_name(DETECTED_PROJECT_NAME)

project(${DETECTED_PROJECT_NAME} LANGUAGES C CXX CSharp)

# Set root directory (one level above Code/)
set_property(GLOBAL PROPERTY NS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(NS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Set build system path relative to NS_ROOT
set_property(GLOBAL PROPERTY NS_CMAKE_RELPATH "Code/BuildSystem/CMake")
set_property(GLOBAL PROPERTY NS_CMAKE_RELPATH_CODE "Code")

# Define build type names (Debug / Dev / Shipping)
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEBUG "Debug")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEV "Dev")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_RELEASE "Shipping")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEBUG_UPPER "DEBUG")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEV_UPPER "DEV")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_RELEASE_UPPER "SHIPPING")
set_property(GLOBAL PROPERTY NS_DEV_BUILD_LINKERFLAGS "RELEASE")

# Include the build system utilities
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/CMake")
include("${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/CMake/nsUtils.cmake")

# Detect platform and configure
include("${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/CMake/nsPlatform.cmake")

# Initialize build types, filters, and external project support
ns_configure_external_project()

# Disable Qt support if Qt6 is not available
find_package(Qt6 QUIET COMPONENTS Core)
if(NOT Qt6_FOUND)
  set(NS_ENABLE_QT_SUPPORT OFF CACHE BOOL "Whether to add Qt support." FORCE)
  message(STATUS "Qt6 not found — disabling Qt support")
endif()

# Write configuration files
ns_write_configuration_txt()

# ThirdParty
add_subdirectory(ThirdParty)

# Foundation (core framework library)
add_subdirectory(Foundation)

# Core Engine
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/CMakeLists.txt")
  add_subdirectory(Engine)
endif()

# Engine Plugins (renderer backends)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/EnginePlugins/CMakeLists.txt")
  add_subdirectory(EnginePlugins)
endif()

# Tools
add_subdirectory(Tools)

# Editor (optional)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Editor/CMakeLists.txt")
  add_subdirectory(Editor)
endif()

# Editor Plugins (optional)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/EditorPlugins/CMakeLists.txt")
  add_subdirectory(EditorPlugins)
endif()

# Samples (optional)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Samples/CMakeLists.txt")
  add_subdirectory(Samples)
endif()

# Unit Tests
add_subdirectory(UnitTests)

# Arbitor GUI
add_subdirectory(Arbitor)