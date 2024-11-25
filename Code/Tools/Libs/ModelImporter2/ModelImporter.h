#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <ModelImporter2/Importer/Importer.h>

namespace nsModelImporter2
{
  NS_MODELIMPORTER2_DLL nsUniquePtr<Importer> RequestImporterForFileType(const char* szFile);

} // namespace nsModelImporter2
