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

# Syncfusion references
set(SYNCFUSION_REFERENCES
    "Syncfusion.BulletGraph.Windows"
    "Syncfusion.Calculate.Base"
    "Syncfusion.Calculate.Windows"
    "Syncfusion.Core.WinForms"
    "Syncfusion.Data.WinForms"
    "Syncfusion.DataSource.WinForms"
    "Syncfusion.Diagram.Base"
    "Syncfusion.Diagram.Windows"
    "Syncfusion.Edit.Windows"
    "Syncfusion.Gauge.Windows"
    "Syncfusion.Grid.Grouping.Windows"
    "Syncfusion.GridCommon.WinForms"
    "Syncfusion.Gridconverter.Windows"
    "Syncfusion.GridHelperClasses.Windows"
    "Syncfusion.HTMLUI.Base"
    "Syncfusion.HTMLUI.windows"
    "Syncfusion.Linq.Base"
    "Syncfusion.Maps.Windows"
    "Syncfusion.MIME.Base"
    "Syncfusion.Pdf.Base"
    "Syncfusion.Tools.WPF"
    "Syncfusion.PdfToImageConverter.Base"
    "Syncfusion.PdfViewer.Windows"
    "Syncfusion.PivotAnalysis.Base"
    "Syncfusion.PivotAnalysis.Windows"
    "Syncfusion.PivotChart.Windows"
    "Syncfusion.Presentation.Base"
    "Syncfusion.ProjIO.Base"
    "Syncfusion.Schedule.Base"
    "Syncfusion.Schedule.Windows"
    "Syncfusion.Scripting.Base"
    "Syncfusion.SfBarcode.Windows"
    "Syncfusion.SfDataGrid.WinForms"
    "Syncfusion.SfInput.WinForms"
    "Syncfusion.SfListView.WinForms"
    "Syncfusion.SfSmithChart.WinForms"
    "Syncfusion.SfSkinManager.WPF"
    "Syncfusion.Shared.WPF"
    "Syncfusion.Spreadsheet.Windows"
    "Syncfusion.TreeMap.Windows"
    "Syncfusion.XlsIO.Base"
)

# Syncfusion packages version
set(SYNCFUSION_VERSION "27.1.58")
set(NS_SYNCFUSION_DIR CACHE PATH "Path to the Syncfusion Binaries. this is needed for the WPF projects, but not for Qt. ex: D:\\Syncfusion\\Essential Studio\\WPF")

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

    # Add references to Syncfusion assemblies
    foreach(REFERENCE ${SYNCFUSION_REFERENCES})
        set(ASSEMBLY_PATH "${PACKAGES_DIR}/${REFERENCE}.${SYNCFUSION_VERSION}/lib/net462/${REFERENCE}.dll")

        target_link_libraries(${PROJECT_NAME} PRIVATE ${ASSEMBLY_PATH})

        set_target_properties(${PROJECT_NAME} PROPERTIES 
        VS_DOTNET_REFERENCES_COPY_LOCAL ON
        VS_DOTNET_REFERENCE "${PACKAGES_DIR}/${REFERENCE}.${SYNCFUSION_VERSION}/lib/net462/${REFERENCE}.dll"
        )
    endforeach()
    message(NOTICE "Globbing All Syncfusion DLLs in: ${NS_SYNCFUSION_DIR}/${SYNCFUSION_VERSION}/Assemblies/4.6.2")
    
    message(NOTICE "Syncfusion SkinManager Binary: ${NS_SYNCFUSION_DIR}/${SYNCFUSION_VERSION}/Assemblies/4.6.2/Syncfusion.SfSkinManager.WPF.dll")

    # Set properties for satellite assemblies and output
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DOTNET_REFERENCES_COPY_LOCAL ON
        VS_DOTNET_REFERENCE_Syncfusion.SfSkinManager.WPF "${NS_SYNCFUSION_DIR}/${SYNCFUSION_VERSION}/Assemblies/4.6.2/Syncfusion.SfSkinManager.WPF.dll"
        VS_DOTNET_REFERENCE_Syncfusion.Shared.WPF "${PACKAGES_DIR}/Syncfusion.Shared.WPF.${SYNCFUSION_VERSION}/lib/net462/Syncfusion.Shared.WPF.dll"
        VS_DOTNET_REFERENCE_Syncfusion.Tools.WPF "${PACKAGES_DIR}/Syncfusion.Tools.WPF.${SYNCFUSION_VERSION}/lib/net462/Syncfusion.Tools.WPF.dll"
    )

    # Add All Libs from Syncfusion in case cmake is stupid.
    # Find all DLL files in the specified directory
    file(GLOB SYNCFUSION_DLLS "${NS_SYNCFUSION_DIR}/${SYNCFUSION_VERSION}/Assemblies/4.6.2/Syncfusion.*.*.dll")

    # Loop through each DLL file
    foreach(dll ${SYNCFUSION_DLLS})
        message(NOTICE "Adding Syncfusion DLL: ${dll}")
        # Get the library name from the file name
        # Add the DLL as a library
        target_link_libraries(${PROJECT_NAME} PUBLIC ${dll})
        set_target_properties(${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${dll})

        # Add as .NET reference (assumes you are using C# or similar)
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_DOTNET_REFERENCE ${dll})
    endforeach()
endfunction(ns_target_wpf_prequesites)
