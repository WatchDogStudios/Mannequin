ns_cmake_init()

ns_requires_desktop()
ns_requires_vulkan()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(SHARED_LIBRARY ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    RendererCore
)

ns_link_target_vulkan(${PROJECT_NAME})

ns_add_output_ns_prefix(${PROJECT_NAME})
