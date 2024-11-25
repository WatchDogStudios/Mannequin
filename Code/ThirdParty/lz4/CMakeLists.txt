

ns_cmake_init()

ns_requires(NS_3RDPARTY_LZ4_SUPPORT)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(SHARED_LIBRARY ${PROJECT_NAME} NO_NS_PREFIX)

target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_LZ4_SUPPORT)

if (NS_COMPILE_ENGINE_AS_DLL AND NS_CMAKE_PLATFORM_WINDOWS)

  target_compile_definitions(${PROJECT_NAME} PRIVATE LZ4_DLL_EXPORT=1)

endif()
