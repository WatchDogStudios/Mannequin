ns_cmake_init()

ns_requires(NS_3RDPARTY_TINYEXR_SUPPORT)

if(NS_CMAKE_PLATFORM_WINDOWS_UWP)
	return()
endif()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(STATIC_LIBRARY ${PROJECT_NAME} NO_NS_PREFIX)

target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_TINYEXR_SUPPORT)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
