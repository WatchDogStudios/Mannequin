# ThirdParty libraries
# Note: flip is header-only (FLIP.h) and does not need a CMake build.
# Note: bc7enc_rdo requires NS_USE_BC7ENC to be enabled.
# Note: Directories without CMakeLists.txt (DirectXCompiler, DirectXMath, Perforce, VHACD) are header-only or prebuilt.

add_subdirectory(enet)
add_subdirectory(lz4)
add_subdirectory(mikktspace)
add_subdirectory(stb)
add_subdirectory(stb_image)
add_subdirectory(tinyexr)
add_subdirectory(tracelogging)
add_subdirectory(zlib)
add_subdirectory(zstd)

if(NS_USE_BC7ENC)
  add_subdirectory(bc7enc_rdo)
endif()
