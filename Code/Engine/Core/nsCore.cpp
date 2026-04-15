// Mannequin Engine Core Library
// Core utilities, platform abstraction, and common types.

#include <cstdint>

namespace ns
{
  // Engine version information
  static constexpr uint32_t VersionMajor = 1;
  static constexpr uint32_t VersionMinor = 0;
  static constexpr uint32_t VersionPatch = 0;

  const char* GetEngineVersion()
  {
    return "1.0.0";
  }
}
