ns_cmake_init()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

# Set the .NET SDK version 
set(DOTNET_SDK_VERSION "8.0.100")
set_property(TARGET ${target} PROPERTY DOTNET_SDK ${DOTNET_SDK_VERSION}) 
set_property(TARGET ${target} PROPERTY DOTNET_TARGET_FRAMEWORK "net8.0")
ns_create_target_cs(LIBRARY ${PROJECT_NAME})


get_cmake_property(BASETHEME ArbitorThemeBase)


ns_target_wpf_prequesites(${PROJECT_NAME})
message(NOTICE "ArbitorTheme: Base: ${BASETHEME}")
include(CSharpUtilities)

csharp_set_designer_cs_properties(
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCES
  	"${BASETHEME}"
	"Microsoft.CSharp"
	"PresentationCore"
	"PresentationFramework"
	"System"
	"System.Core"
	"System.Data"
	"System.Data.DataSetExtensions"
	"System.Net.Http"
	"System.Xaml"
	"System.Xml"
	"System.Xml.Linq"
	"Microsoft.VisualStudio.Composition"
	"Microsoft.VisualStudio.Composition.Analyzers"
	"Microsoft.VisualStudio.Composition.AppHost"
	"${CMAKE_CURRENT_BINARY_DIR}/packages/FontAwesome5.2.1.11/lib/net462/FontAwesome5.dll"
	"${CMAKE_CURRENT_BINARY_DIR}/packages/CommunityToolkit.Mvvm.8.2.2/lib/netstandard2.0/CommunityToolkit.Mvvm.dll"
	"${CMAKE_CURRENT_BINARY_DIR}/packages/AdonisUI.1.17.1/lib/net45/AdonisUI.dll"
	"CommunityToolkit.Mvvm"
	"Dirkster.AvalonDock"
	"System.ComponentModel.Composition"
	"WindowsBase")

add_custom_command(TARGET ${PROJECT_NAME}
	PRE_BUILD
	COMMAND ${NUGET} restore ${CMAKE_CURRENT_SOURCE_DIR}/packages.config -PackagesDirectory ${CMAKE_CURRENT_BINARY_DIR}/packages
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
