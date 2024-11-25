#include "ImporterOFBX.h"



nsResult nsModelImporter2::nsFBXImporter::DoImport()
{
  return NS_SUCCESS;
}

nsResult nsModelImporter2::nsFBXImporter::TraverseSkeletonHierarchy(ofbx::IScene* pScene, ofbx::Object* pObject, nsEditableSkeletonJoint* pParentJoint)
{

    return NS_SUCCESS;
}

nsResult nsModelImporter2::nsFBXImporter::PrepareOutputMeshes(ofbx::IScene* pScene)
{
  return NS_SUCCESS;
}

nsResult nsModelImporter2::nsFBXImporter::RecomputeNormals()
{
  return NS_SUCCESS;
}

nsResult nsModelImporter2::nsFBXImporter::ImportMaterials(ofbx::IScene* pScene)
{
  return NS_SUCCESS;
}

nsResult nsModelImporter2::nsFBXImporter::ImportAnimations(ofbx::IScene* pScene)
{
  return NS_SUCCESS;
}

void nsModelImporter2::nsFBXImporter::SimplifyMesh(ofbx::Mesh* pMesh)
{
}

nsColor nsModelImporter2::ConvertOFBXType(const ofbx::Color& value, bool bInvert)
{
  return nsColor();
}

nsMat4 nsModelImporter2::ConvertOFBXType(const ofbx::Matrix& value, bool bDummy)
{
  return nsMat4();
}

nsVec3 nsModelImporter2::ConvertOFBXType(const ofbx::Vec3& value, bool bDummy)
{
  return nsVec3();
}

nsQuat nsModelImporter2::ConvertOFBXType(const ofbx::Quat& value, bool bDummy)
{
  return nsQuat();
}

float nsModelImporter2::ConvertOFBXType(float value, bool bDummy)
{
  return 0.0f;
}

int nsModelImporter2::ConvertOFBXType(int value, bool bDummy)
{
  return 0;
}
