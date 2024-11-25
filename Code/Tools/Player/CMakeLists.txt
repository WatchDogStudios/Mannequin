ns_cmake_init()

ns_requires_renderer()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(APPLICATION ${PROJECT_NAME})
ns_make_windowapp(${PROJECT_NAME})

ns_add_output_ns_prefix(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GameEngine
)

if (NS_CMAKE_PLATFORM_WINDOWS_UWP)
  # Due to app sandboxing we need to explicitly name required plugins for UWP.
  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ParticlePlugin
    InspectorPlugin
    TypeScriptPlugin
  )

  if (NS_BUILD_PHYSX)
    target_link_libraries(${PROJECT_NAME} PUBLIC PhysXPlugin)
  endif()

  if (NS_BUILD_FMOD)
    target_link_libraries(${PROJECT_NAME} PUBLIC FmodPlugin)
  endif()

  if (NS_BUILD_OPENXR)
    find_package(nsOpenXR REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenXRPlugin)
  endif()

endif()

ns_add_renderers(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME}
  InspectorPlugin
)
