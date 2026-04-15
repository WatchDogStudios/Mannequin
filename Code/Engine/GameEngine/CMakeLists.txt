ns_cmake_init()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(SHARED_LIBRARY ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  Core
  RendererCore
)
