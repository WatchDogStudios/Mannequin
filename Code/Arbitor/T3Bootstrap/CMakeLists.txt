ns_cmake_init()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target_cs(APPLICATION ${PROJECT_NAME})

get_cmake_property(BASETHEME ArbitorThemeBase)


include(CSharpUtilities)

csharp_set_designer_cs_properties(
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings)

csharp_set_xaml_cs_properties(
	App.xaml
	App.xaml.cs
	MainWindow.xaml
	MainWindow.xaml.cs)

set_property(SOURCE App.xaml PROPERTY VS_XAML_TYPE "ApplicationDefinition")

#ns_make_winmain_executable(${PROJECT_NAME})

# Set the .NET SDK version 
set(DOTNET_SDK_VERSION "8.0.100")
set_property(TARGET ${target} PROPERTY DOTNET_SDK ${DOTNET_SDK_VERSION}) 
set_property(TARGET ${target} PROPERTY DOTNET_TARGET_FRAMEWORK "net8.0")

target_link_libraries(${PROJECT_NAME} PUBLIC T3Foundation)

ns_target_wpf_prequesites(${PROJECT_NAME})

message(NOTICE "ArbitorTheme: Base: ${BASETHEME}")
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
	"Microsoft.CSharp"
	"System.Drawing"
	"System.Windows.Forms"
	"System.ComponentModel.Composition"
	"System.ComponentModel.Composition.Hosting"
	"WindowsBase")
