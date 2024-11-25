#pragma once

#include <ModelImporter2/Importer/Importer.h>

namespace nsModelImporter2
{
  /// Importer implementation to import Source engine BSP files.
  class ImporterMagicaVoxel : public Importer
  {
  public:
    ImporterMagicaVoxel();
    ~ImporterMagicaVoxel();

  protected:
    virtual nsResult DoImport() override;
  };
} // namespace nsModelImporter2
