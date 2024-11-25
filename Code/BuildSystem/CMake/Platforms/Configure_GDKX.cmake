# This is a NDA Protected Platform. The Developers GDK should already be signed with Microsoft.
# NOTE(Mikael A.): We can leave the cmake specific code here, but any API code should be requested/left on the gitlab version.

include("${CMAKE_CURRENT_LIST_DIR}/Configure_Default.cmake")

message(STATUS "Configuring NDA Platform: Scarlett (Xbox Series X)")

set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_XBOX_SERIES_X ON)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_CONSOLE ON)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_VULKAN OFF)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_EDITOR OFF)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSIX OFF)

# TODO: Consider Build Without Install.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OPTIMIZE_FOR_SCARLETT "Optimize for the AMD Hercules CPU" ON)
option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

# #####################################
# ## General settings
# #####################################
set(NS_COMPILE_ENGINE_AS_DLL ON CACHE BOOL "Whether to compile the code as a shared libraries (DLL).")
mark_as_advanced(FORCE NS_COMPILE_ENGINE_AS_DLL)

macro(ns_platform_pull_properties)
    get_property(NS_CMAKE_PLATFORM_XBOX_SERIES_X GLOBAL PROPERTY NS_CMAKE_PLATFORM_XBOX_SERIES_X)
    get_property(NS_CMAKE_PLATFORM_CONSOLE GLOBAL PROPERTY NS_CMAKE_PLATFORM_CONSOLE)
    get_property(NS_CMAKE_PLATFORM_SUPPORTS_VULKAN GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_VULKAN)
    get_property(NS_CMAKE_PLATFORM_SUPPORTS_EDITOR GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_EDITOR)
    get_property(NS_CMAKE_PLATFORM_POSIX GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSIX)
endmacro()

macro(ns_platformhook_set_build_flags_clang)

    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)

    # Disable the warning that clang doesn't support pragma optimize.
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-ignored-pragma-optimize -Wno-pragma-pack)

    # -march=btver2 to target AMD Jaguar CPU; -march=znver2 to target AMD Hercules CPU (requires clang v9; otherwise use znver1)
    set(Console_ArchOptions -march=$<IF:$<BOOL:${OPTIMIZE_FOR_SCARLETT}>,$<IF:$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,9.0>,znver2,znver1>,btver2>)

    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-gnu-anonymous-struct
        -Wno-language-extension-token
        -Wno-nested-anon-types
        -Wno-reserved-id-macro
        -Wno-unknown-pragmas)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-reserved-identifier)
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unsafe-buffer-usage)
    endif()

    target_link_options(${PROJECT_NAME} PRIVATE /IGNORE:4078)
endmacro()

macro(ns_platform_detect_generator)
    string(FIND ${CMAKE_VERSION} "MSVC" VERSION_CONTAINS_MSVC)

    if(${VERSION_CONTAINS_MSVC} GREATER -1)
        message(STATUS "CMake was called from Visual Studio Open Folder workflow")
        set_property(GLOBAL PROPERTY NS_CMAKE_INSIDE_VS ON)
    endif()

    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        # Visual Studio (All VS generators define MSVC)
        message(STATUS "Generator is MSVC (NS_CMAKE_GENERATOR_MSVC)")

        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_MSVC ON)
        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX "Vs")
        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION $<CONFIGURATION>)
    elseif(CMAKE_GENERATOR MATCHES "Ninja") # Ninja makefiles. Only makefile format supported by Visual Studio Open Folder
        message(STATUS "Buildsystem is Ninja (NS_CMAKE_GENERATOR_NINJA)")

        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_NINJA ON)
        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX "Ninja")
        set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})
    else()
        message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Windows! Please extend ns_platform_detect_generator()")
    endif()
endmacro()