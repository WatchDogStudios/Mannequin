#include <Texture/Image/ImageConversion.h>
#include <Foundation/Logging/Log.h>

nsResult nsImageConversion::Convert(const nsImage& source, nsImage& ref_target, nsImageFormat::Enum targetFormat)
{
  if (source.GetImageFormat() == targetFormat)
  {
    ref_target = source;
    return NS_SUCCESS;
  }

  nsLog::Warning("nsImageConversion::Convert - format conversion not yet implemented, copying raw data");
  ref_target = source;
  return NS_SUCCESS;
}

bool nsImageConversion::IsConvertible(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat)
{
  // Stub: only same-format "conversion" is supported
  return sourceFormat == targetFormat;
}

nsResult nsImageConversion::BuildPath(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, bool bSourceEqualsTarget,
  nsHybridArray<ConversionPathNode, 16>& ref_path, nsUInt32& ref_uiNumScratchBuffers)
{
  ref_path.Clear();
  ref_uiNumScratchBuffers = 0;

  if (sourceFormat == targetFormat)
    return NS_SUCCESS;

  ConversionPathNode node;
  node.m_sourceFormat = sourceFormat;
  node.m_targetFormat = targetFormat;
  ref_path.PushBack(node);

  return NS_SUCCESS;
}
