#include <ModelImporter2/ModelImporterPCH.h>

#include <Foundation/Logging/Log.h>
#include <ModelImporter2/Importer/Importer.h>
#include <RendererCore/AnimationSystem/EditableSkeleton.h>
#include <RendererCore/Meshes/MeshResourceDescriptor.h>

namespace nsModelImporter2
{
  Importer::Importer() = default;
  Importer::~Importer() = default;

  nsResult Importer::Import(const ImportOptions& options, nsLogInterface* pLogInterface /*= nullptr*/, nsProgress* pProgress /*= nullptr*/)
  {
    nsResult res = NS_FAILURE;

    nsLogInterface* pPrevLogSystem = nsLog::GetThreadLocalLogSystem();

    if (pLogInterface)
    {
      nsLog::SetThreadLocalLogSystem(pLogInterface);
    }

    {
      m_pProgress = pProgress;
      m_Options = options;

      NS_LOG_BLOCK("ModelImport", m_Options.m_sSourceFile);

      res = DoImport();
    }


    nsLog::SetThreadLocalLogSystem(pPrevLogSystem);

    return res;
  }

  void OutputTexture::GenerateFileName(nsStringBuilder& out_sName) const
  {
    nsStringBuilder tmp("Embedded_", m_sFilename);

    nsPathUtils::MakeValidFilename(tmp.GetFileName(), '_', out_sName);
    out_sName.ChangeFileExtension(m_sFileFormatExtension);
  }

} // namespace nsModelImporter2
