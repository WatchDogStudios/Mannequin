function(find_xbmgr_exe OUTPUT_VARIABLE)
    find_program(XBMGR_EXECUTABLE
        NAMES xbmgr.exe
        PATHS
            /path/to/search/directory1
            /path/to/search/directory2
            # Add more directories as needed
    )

    if(NOT XBMGR_EXECUTABLE)
        message(FATAL_ERROR "Could not find xbmgr.exe")
    endif()

    set(${OUTPUT_VARIABLE} ${XBMGR_EXECUTABLE} PARENT_SCOPE)
endfunction()