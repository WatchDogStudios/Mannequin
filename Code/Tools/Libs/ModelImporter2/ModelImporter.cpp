#include <ModelImporter2/ModelImporterPCH.h>

#include <ModelImporter2/ImporterAssimp/ImporterAssimp.h>
#include <ModelImporter2/ImporterMagicaVoxel/ImporterMagicaVoxel.h>
#include <ModelImporter2/ImporterSourceBSP/ImporterSourceBSP.h>
#include <ModelImporter2/ModelImporter.h>

namespace nsModelImporter2
{

  nsUniquePtr<Importer> RequestImporterForFileType(const char* szFile)
  {
    nsStringBuilder sFile = szFile;

    if (sFile.HasExtension(".fbx") || sFile.HasExtension(".obj") || sFile.HasExtension(".gltf") || sFile.HasExtension(".glb") || sFile.HasExtension(".blend"))
    {
      return NS_DEFAULT_NEW(ImporterAssimp);
    }

    if (sFile.HasExtension(".bsp"))
    {
      return NS_DEFAULT_NEW(ImporterSourceBSP);
    }

    if (sFile.HasExtension(".vox"))
    {
      return NS_DEFAULT_NEW(ImporterMagicaVoxel);
    }

    return nullptr;
  }


} // namespace nsModelImporter2
