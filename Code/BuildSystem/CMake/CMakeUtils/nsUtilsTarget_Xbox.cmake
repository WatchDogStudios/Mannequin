# include CMAKEGDKXBOX, does most of the hard work for us.
include("Code/BuildSystem/CMake/Platforms/Xbox/gxdk_xs_toolchain.cmake")
include("Code/BuildSystem/CMake/Platforms/Xbox/CMakeGDKXbox.cmake")

function(ns_xboxseries_add_target_configurations TARGET_NAME)
    if(NS_GDKX_BUILDFOR_XBOX_SERIES_X)
            # Scarrlet Optimizations.
            set(Console_ArchOptions /favor:AMD64 $<IF:$<BOOL:${OPTIMIZE_FOR_SCARLETT}>,/arch:AVX2,/arch:AVX>)

            if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
                target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Gw>)

                if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.35)
                    target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Zc:checkGwOdr>)
                endif()

                target_link_options(${PROJECT_NAME} PRIVATE /IGNORE:4075 ${Console_ArchOptions_LTCG})
            endif()

            # /ZH:SHA_256 (secure source hashing)
            if((MSVC_VERSION GREATER_EQUAL 1924)
                AND((NOT(CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)))
                target_compile_options(${PROJECT_NAME} PRIVATE /ZH:SHA_256)
            endif()

            if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
                target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /Zc:__cplusplus /Zc:inline)

                if(ENABLE_CODE_ANALYSIS)
                    target_compile_options(${PROJECT_NAME} PRIVATE /analyze)
                endif()

                if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
                    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:preprocessor)
                endif()

                if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28)
                    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:lambda)
                endif()

                if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
                    target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Gw>)

                    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.35)
                        target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Zc:checkGwOdr>)
                    endif()

                    target_link_options(${PROJECT_NAME} PRIVATE /IGNORE:4075 ${Console_ArchOptions_LTCG})
                endif()
            endif()

            target_compile_definitions(${TARGET_NAME} PUBLIC _GAMING_XBOX_SCARLETT=1 _XBOX_SERIES_=1)

            target_include_directories(${PROJECT_NAME} PUBLIC ${Console_SdkIncludeRoot})
            target_link_directories(${PROJECT_NAME} PUBLIC ${VC_OneCore_LibPath} PUBLIC ${Console_SdkLibPath})

            # See the CMakeExample in Xbox-GDK-Samples for more information
            target_compile_definitions(${PROJECT_NAME} PRIVATE ${Console_Defines})

            # TODO: Utility functions to add the required libraries with only the name.
            target_link_libraries(${PROJECT_NAME} PRIVATE ${Console_Libs})

            # optional extension libraries: Xbox::XSAPI Xbox::XCurl Xbox::GameChat2 Xbox::PlayFabMultiplayer Xbox::PlayFabParty Xbox::PlayFabPartyLIVE Xbox::PlayFabServices
            target_link_options(${PROJECT_NAME} PRIVATE ${Console_LinkOptions})

            set(GAME_ASSETS
                Assets/Logo.png
                Assets/SmallLogo.png
                Assets/SplashScreen.png
                Assets/StoreLogo.png
            )

            source_group("Assets" FILES ${GAME_ASSETS})

            # MicrosoftGameConfig.mgc
            set_source_files_properties(MicrosoftGameConfig.mgc PROPERTIES VS_TOOL_OVERRIDE "MGCCompile")
            set_source_files_properties(Assets/Logo.png PROPERTIES VS_TOOL_OVERRIDE "CopyFileToFolders")
            set_source_files_properties(Assets/SmallLogo.png PROPERTIES VS_TOOL_OVERRIDE "CopyFileToFolders")
            set_source_files_properties(Assets/SplashScreen.png PROPERTIES VS_TOOL_OVERRIDE "CopyFileToFolders")
            set_source_files_properties(Assets/StoreLogo.png PROPERTIES VS_TOOL_OVERRIDE "CopyFileToFolders")

            # Copy VC Runtime and Extension Library DLLs
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CppRuntimeFilesPath}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )

            # Copy Microsoft Game Config
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CppRuntimeFilesPath}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )

            # Command To Compile Config
            # TODO: Move this over to a seprate csharp executable.
            #add_custom_command(TARGET mgccompile
            #    COMMAND ${MAKEPKG_TOOL} localize /d "${CMAKE_BINARY_DIR}" /pd "${CMAKE_BINARY_DIR}/bin/Gaming.Xbox.Scarlett.x64"
            #    MAIN_DEPENDENCY $<TARGET_FILE:${PROJECT_NAME}/MicrosoftGameConfig.mgc>
            #    COMMENT "makepkg localize MicrosoftGame.config"
            #    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            #    VERBATIM)
    endif()
endfunction()