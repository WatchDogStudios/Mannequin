add_subdirectory(ThirdParty)

# Core Engine
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/CMakeLists.txt")
  add_subdirectory(Engine)
endif()

# Engine Plugins (renderer backends)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/EnginePlugins/CMakeLists.txt")
  add_subdirectory(EnginePlugins)
endif()

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

add_subdirectory(UnitTests)
add_subdirectory(Arbitor)