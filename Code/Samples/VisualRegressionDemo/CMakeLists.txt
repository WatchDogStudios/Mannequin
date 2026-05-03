ns_cmake_init()

ns_requires_renderer()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    RendererCore
)

ns_add_renderers(${PROJECT_NAME})
