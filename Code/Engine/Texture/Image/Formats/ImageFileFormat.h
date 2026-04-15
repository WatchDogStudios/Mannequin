#pragma once

#include <Texture/TextureDLL.h>
#include <Foundation/Strings/String.h>

class nsImage;
class nsStreamReader;
class nsStreamWriter;

/// Base class for image file format handlers
class NS_TEXTURE_DLL nsImageFileFormat
{
public:
  virtual ~nsImageFileFormat() = default;

  /// Read image from stream
  virtual nsResult ReadImage(nsStreamReader& inout_stream, nsImage& ref_image, nsStringView sFileExtension) const = 0;

  /// Write image to stream
  virtual nsResult WriteImage(nsStreamWriter& inout_stream, const nsImage& image, nsStringView sFileExtension) const = 0;

  /// Check if this format handler supports the given extension
  virtual bool CanReadFileType(nsStringView sExtension) const = 0;
  virtual bool CanWriteFileType(nsStringView sExtension) const = 0;

  /// Get a format handler for the given file extension
  static nsImageFileFormat* GetReaderFormat(nsStringView sExtension);
  static nsImageFileFormat* GetWriterFormat(nsStringView sExtension);
};
