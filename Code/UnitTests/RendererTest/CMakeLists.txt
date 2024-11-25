ns_cmake_init()

ns_requires_renderer()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  TestFramework
  RendererCore
)

if(NS_CMAKE_PLATFORM_ANDROID)
    #TODO: Add actual packaging code. This is done in PRE_BUILD so that it happens before the
    #apk gen steps that happen in POST_BUILD and which are already done via ns_create_target.
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${NS_ROOT}/Output/ShaderCache/VULKAN/ ${CMAKE_CURRENT_BINARY_DIR}/package/Assets/Output/ShaderCache/VULKAN/)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${NS_ROOT}/Data/UnitTests/RendererTest/ ${CMAKE_CURRENT_BINARY_DIR}/package/Assets/Data/UnitTests/RendererTest/)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${NS_ROOT}/Data/UnitTests/SharedData/ ${CMAKE_CURRENT_BINARY_DIR}/package/Assets/Data/UnitTests/SharedData/)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${NS_ROOT}/Data/Base/Shaders/ ${CMAKE_CURRENT_BINARY_DIR}/package/Assets/Data/Base/Shaders/)
endif()

ns_add_renderers(${PROJECT_NAME})

ns_ci_add_test(${PROJECT_NAME} NEEDS_HW_ACCESS)

