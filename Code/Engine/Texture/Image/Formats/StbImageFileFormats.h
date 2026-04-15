#pragma once

#include <Texture/Image/Formats/ImageFileFormat.h>

/// STB-based image file format handler (PNG, JPG, TGA, etc.)
class NS_TEXTURE_DLL nsStbImageFileFormats : public nsImageFileFormat
{
public:
  virtual nsResult ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const override;
  virtual nsResult WriteImage(nsStreamWriter& inout_stream, const nsImage& image, nsStringView sFileExtension) const override;
  virtual bool CanReadFileType(nsStringView sExtension) const override;
  virtual bool CanWriteFileType(nsStringView sExtension) const override;
};
