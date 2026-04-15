#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Foundation/Logging/Log.h>

nsResult nsDdsFileFormat::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsDdsFileFormat::ReadImage not yet implemented");
  return NS_FAILURE;
}

nsResult nsDdsFileFormat::WriteImage(nsStreamWriter& inout_stream, const nsImage& image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsDdsFileFormat::WriteImage not yet implemented");
  return NS_FAILURE;
}

bool nsDdsFileFormat::CanReadFileType(nsStringView sExtension) const
{
  return sExtension == "dds";
}

bool nsDdsFileFormat::CanWriteFileType(nsStringView sExtension) const
{
  return sExtension == "dds";
}
