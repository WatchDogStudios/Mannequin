#include <Texture/Image/Formats/BmpFileFormat.h>
#include <Foundation/Logging/Log.h>

nsResult nsBmpFileFormat::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsBmpFileFormat::ReadImage not yet implemented");
  return NS_FAILURE;
}

nsResult nsBmpFileFormat::WriteImage(nsStreamWriter& inout_stream, const nsImage& image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsBmpFileFormat::WriteImage not yet implemented");
  return NS_FAILURE;
}

bool nsBmpFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension == "bmp";
}

bool nsBmpFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  return sExtension == "bmp";
}
