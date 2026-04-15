#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Strings/String.h>

class nsEditableSkeletonJoint;

/// \brief Editable skeleton for animation import/editing.
class NS_RENDERERCORE_DLL nsEditableSkeleton
{
public:
  nsEditableSkeleton();
  ~nsEditableSkeleton();

  nsDynamicArray<nsEditableSkeletonJoint*> m_Children;
};

/// \brief A single joint in an editable skeleton.
class NS_RENDERERCORE_DLL nsEditableSkeletonJoint
{
public:
  nsString m_sName;
  nsTransform m_Transform;
  nsDynamicArray<nsEditableSkeletonJoint*> m_Children;
};
