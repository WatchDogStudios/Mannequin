#pragma once

#include <ModelImporter2/Importer/Importer.h>

#include <OpenFBX/ofbx.h>

class nsEditableSkeletonJoint;

namespace nsModelImporter2
{
  class nsFBXImporter : public Importer
  {
  public:
    nsFBXImporter();
    ~nsFBXImporter();

    virtual nsResult DoImport() override;

  private:
    nsResult TraverseSkeletonHierarchy(ofbx::IScene* pScene, ofbx::Object* pObject, nsEditableSkeletonJoint* pParentJoint);

    nsResult PrepareOutputMeshes(ofbx::IScene* pScene);
    nsResult RecomputeNormals();

    nsResult ImportMaterials(ofbx::IScene* pScene);
    nsResult ImportAnimations(ofbx::IScene* pScene);

    void SimplifyMesh(ofbx::Mesh* pMesh);
    nsUInt32 m_uiTotalMeshVertices = 0;
    nsUInt32 m_uiTotalMeshTriangles = 0;
    struct MeshInstance
    {
      nsMat4 m_GlobalTransform;
      ofbx::Mesh* m_pMesh;
    };
    nsMap<nsUInt32, nsHybridArray<MeshInstance, 4>> m_MeshInstances;
    nsSet<ofbx::Mesh*> m_OptimizedMeshes;
  };
  extern nsColor ConvertOFBXType(const ofbx::Color& value, bool bInvert = false);
  extern nsMat4 ConvertOFBXType(const ofbx::Matrix& value, bool bDummy = false);
  extern nsVec3 ConvertOFBXType(const ofbx::Vec3& value, bool bDummy = false);
  extern nsQuat ConvertOFBXType(const ofbx::Quat& value, bool bDummy = false);
  extern float ConvertOFBXType(float value, bool bDummy = false);
  extern int ConvertOFBXType(int value, bool bDummy = false);

} // namespace nsModelImporter2
