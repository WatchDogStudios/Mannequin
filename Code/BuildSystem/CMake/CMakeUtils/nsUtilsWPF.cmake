# Basic system references
set(SYSTEM_REFERENCES
    "System"
    "System.Core"
    "System.Data"
    "System.Data.DataSetExtensions"
    "System.Drawing"
    "System.Net.Http"
    "System.Windows.Forms"
    "System.Xml"
    "System.Xml.Linq"
    "Microsoft.CSharp"
    "System.ComponentModel.Composition"
    "System.ComponentModel.Composition.Hosting"
)

# WPF related references
set(WPF_REFERENCES
    "PresentationCore"
    "PresentationFramework"
    "System.Xaml"
    "System.Printing"
    "ReachFramework"
)

# Syncfusion references — WPF assemblies used by Arbitor (from local install)
set(SYNCFUSION_REFERENCES
    "Syncfusion.SfSkinManager.WPF"
    "Syncfusion.Shared.WPF"
    "Syncfusion.Tools.WPF"
    "Syncfusion.Licensing"
)

# Syncfusion packages version
set(SYNCFUSION_VERSION "33.1.44")
set(NS_SYNCFUSION_DIR "C:/Program Files (x86)/Syncfusion/Essential Studio/WPF" CACHE PATH "Path to the Syncfusion Binaries. this is needed for the WPF projects, but not for Qt. ex: D:\\Syncfusion\\Essential Studio\\WPF")

# Function to find and add Syncfusion DLLs
function(ns_add_syncfusion_dlls syncfusion_dir TARGET)
    
endfunction()


function(ns_target_wpf_prequesites PROJECT_NAME)
    # Add references to system assemblies
    foreach(REFERENCE ${SYSTEM_REFERENCES})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${REFERENCE})
    endforeach()

    # Add references to WPF assemblies
    foreach(REFERENCE ${WPF_REFERENCES})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${REFERENCE})
    endforeach()

    set(SYNCFUSION_ASSEMBLY_DIR "${NS_SYNCFUSION_DIR}/${SYNCFUSION_VERSION}/Assemblies/4.6.2")
    message(NOTICE "Syncfusion assembly dir: ${SYNCFUSION_ASSEMBLY_DIR}")

    # Add references to Syncfusion assemblies from local install
    foreach(REFERENCE ${SYNCFUSION_REFERENCES})
        set(ASSEMBLY_PATH "${SYNCFUSION_ASSEMBLY_DIR}/${REFERENCE}.dll")
        if(EXISTS "${ASSEMBLY_PATH}")
            target_link_libraries(${PROJECT_NAME} PRIVATE "${ASSEMBLY_PATH}")
            message(NOTICE "  Linked Syncfusion: ${REFERENCE}")
        else()
            message(WARNING "Syncfusion assembly not found: ${ASSEMBLY_PATH}")
        endif()
    endforeach()

    message(NOTICE "Syncfusion SkinManager Binary: ${SYNCFUSION_ASSEMBLY_DIR}/Syncfusion.SfSkinManager.WPF.dll")

    # Set VS_DOTNET_REFERENCE properties for key assemblies
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DOTNET_REFERENCES_COPY_LOCAL ON
        VS_DOTNET_REFERENCE_Syncfusion.SfSkinManager.WPF "${SYNCFUSION_ASSEMBLY_DIR}/Syncfusion.SfSkinManager.WPF.dll"
        VS_DOTNET_REFERENCE_Syncfusion.Shared.WPF "${SYNCFUSION_ASSEMBLY_DIR}/Syncfusion.Shared.WPF.dll"
        VS_DOTNET_REFERENCE_Syncfusion.Tools.WPF "${SYNCFUSION_ASSEMBLY_DIR}/Syncfusion.Tools.WPF.dll"
    )

    # Glob all Syncfusion WPF DLLs from the install directory as additional references
    file(GLOB SYNCFUSION_DLLS "${SYNCFUSION_ASSEMBLY_DIR}/Syncfusion.*.dll")
    foreach(dll ${SYNCFUSION_DLLS})
        target_link_libraries(${PROJECT_NAME} PUBLIC "${dll}")
    endforeach()
endfunction(ns_target_wpf_prequesites)
