#include <ModelImporter2/ModelImporterPCH.h>

#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/types.h>
#include <assimp/vector3.h>

namespace nsModelImporter2
{
  nsColor ConvertAssimpType(const aiColor4D& value, bool bInvert /*= false*/)
  {
    if (bInvert)
      return nsColor(1.0f - value.r, 1.0f - value.g, 1.0f - value.b, 1.0f - value.a);
    else
      return nsColor(value.r, value.g, value.b, value.a);
  }

  nsColor ConvertAssimpType(const aiColor3D& value, bool bInvert /*= false*/)
  {
    if (bInvert)
      return nsColor(1.0f - value.r, 1.0f - value.g, 1.0f - value.b);
    else
      return nsColor(value.r, value.g, value.b);
  }

  nsMat4 ConvertAssimpType(const aiMatrix4x4& value, bool bDummy /*= false*/)
  {
    NS_ASSERT_DEBUG(!bDummy, "not implemented");

    return nsMat4::MakeFromRowMajorArray(&value.a1);
  }

  nsVec3 ConvertAssimpType(const aiVector3D& value, bool bDummy /*= false*/)
  {
    NS_ASSERT_DEBUG(!bDummy, "not implemented");

    return nsVec3(value.x, value.y, value.z);
  }

  nsQuat ConvertAssimpType(const aiQuaternion& value, bool bDummy /*= false*/)
  {
    NS_ASSERT_DEBUG(!bDummy, "not implemented");

    return nsQuat(value.x, value.y, value.z, value.w);
  }

  float ConvertAssimpType(float value, bool bDummy /*= false*/)
  {
    NS_ASSERT_DEBUG(!bDummy, "not implemented");

    return value;
  }

  int ConvertAssimpType(int value, bool bDummy /*= false*/)
  {
    NS_ASSERT_DEBUG(!bDummy, "not implemented");

    return value;
  }

} // namespace nsModelImporter2
