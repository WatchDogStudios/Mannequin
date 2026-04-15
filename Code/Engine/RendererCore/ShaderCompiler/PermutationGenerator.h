#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Strings/HashedString.h>

/// \brief A permutation variable used in shader compilation.
struct nsPermutationVar
{
  nsHashedString m_sName;
  nsHashedString m_sValue;
};

/// \brief Generates all permutations from a set of variables and their values.
class NS_RENDERERCORE_DLL nsPermutationGenerator
{
public:
  void Clear();

  void AddPermutation(const nsHashedString& sName, const nsHashedString& sValue);
  void RemovePermutations(const nsHashedString& sName);

  nsUInt32 GetPermutationCount() const;
  void GetPermutation(nsUInt32 uiIndex, nsHybridArray<nsPermutationVar, 16>& out_permVars) const;
};
