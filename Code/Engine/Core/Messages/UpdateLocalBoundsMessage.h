#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Math/BoundingBoxSphere.h>

/// \brief Spatial data categories.
struct nsSpatialData
{
  struct Category
  {
    nsUInt32 m_uiValue = 0;
  };

  static Category RegisterCategory(const char* szName, const nsBitflags<nsObjectFlags>& flags = {});
};

/// \brief Default spatial data categories.
namespace nsDefaultSpatialDataCategories
{
  NS_CORE_DLL extern nsSpatialData::Category RenderStatic;
  NS_CORE_DLL extern nsSpatialData::Category RenderDynamic;
}

/// \brief Flags for spatial objects.
struct nsObjectFlags
{
  using StorageType = nsUInt32;
  enum Enum : nsUInt32
  {
    None = 0,
    Default = None
  };
};

/// \brief Message sent to update local bounds of an object.
struct NS_CORE_DLL nsMsgUpdateLocalBounds : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgUpdateLocalBounds, nsMessage);

  void AddBounds(const nsBoundingBoxSphere& bounds, nsSpatialData::Category category);
};
