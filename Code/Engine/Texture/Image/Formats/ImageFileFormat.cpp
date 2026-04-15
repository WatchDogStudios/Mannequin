#include <Texture/Image/Formats/ImageFileFormat.h>
#include <Foundation/Logging/Log.h>

nsImageFileFormat* nsImageFileFormat::GetReaderFormat(nsStringView sExtension)
{
  nsLog::Warning("nsImageFileFormat::GetReaderFormat not yet implemented");
  return nullptr;
}

nsImageFileFormat* nsImageFileFormat::GetWriterFormat(nsStringView sExtension)
{
  nsLog::Warning("nsImageFileFormat::GetWriterFormat not yet implemented");
  return nullptr;
}
