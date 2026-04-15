#include <Texture/Image/Formats/StbImageFileFormats.h>
#include <Foundation/Logging/Log.h>

nsResult nsStbImageFileFormats::ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsStbImageFileFormats::ReadImage not yet implemented");
  return NS_FAILURE;
}

nsResult nsStbImageFileFormats::WriteImage(nsStreamWriter& inout_stream, const nsImage& image, nsStringView sFileExtension) const
{
  nsLog::Warning("nsStbImageFileFormats::WriteImage not yet implemented");
  return NS_FAILURE;
}

bool nsStbImageFileFormats::CanReadFileType(nsStringView sExtension) const
{
  return sExtension == "png" || sExtension == "jpg" || sExtension == "jpeg" || sExtension == "tga" || sExtension == "hdr" || sExtension == "gif";
}

bool nsStbImageFileFormats::CanWriteFileType(nsStringView sExtension) const
{
  return sExtension == "png" || sExtension == "jpg" || sExtension == "jpeg" || sExtension == "tga" || sExtension == "hdr";
}
