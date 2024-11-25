#pragma once

#include <ModelImporter2/Importer/Importer.h>

namespace nsModelImporter2
{
  /// Importer implementation to import Source engine BSP files.
  class ImporterSourceBSP : public Importer
  {
  public:
    ImporterSourceBSP();
    ~ImporterSourceBSP();

  protected:
    virtual nsResult DoImport() override;
  };
} // namespace nsModelImporter2
