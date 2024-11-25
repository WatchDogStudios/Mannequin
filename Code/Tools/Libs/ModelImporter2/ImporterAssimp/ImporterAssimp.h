#pragma once

#include <ModelImporter2/Importer/Importer.h>

#include <assimp/Importer.hpp>

class nsEditableSkeletonJoint;
struct aiNode;
struct aiMesh;

namespace nsModelImporter2
{
  class ImporterAssimp : public Importer
  {
  public:
    ImporterAssimp();
    ~ImporterAssimp();

  protected:
    virtual nsResult DoImport() override;

  private:
    nsResult TraverseAiScene();

    nsResult PrepareOutputMesh();
    nsResult RecomputeTangents();

    nsResult TraverseAiNode(aiNode* pNode, const nsMat4& parentTransform, nsEditableSkeletonJoint* pCurJoint);
    nsResult ProcessAiMesh(aiMesh* pMesh, const nsMat4& transform);

    nsResult ImportMaterials();
    nsResult ImportAnimations();

    nsResult ImportBoneColliders(nsEditableSkeletonJoint* pJoint);

    void SimplifyAiMesh(aiMesh* pMesh);

    Assimp::Importer m_Importer;
    const aiScene* m_pScene = nullptr;
    nsUInt32 m_uiTotalMeshVertices = 0;
    nsUInt32 m_uiTotalMeshTriangles = 0;

    struct MeshInstance
    {
      nsMat4 m_GlobalTransform;
      aiMesh* m_pMesh;
    };

    nsMap<nsUInt32, nsHybridArray<MeshInstance, 4>> m_MeshInstances;

    nsSet<aiMesh*> m_OptimizedMeshes;
  };

  extern nsColor ConvertAssimpType(const aiColor3D& value, bool bInvert = false);
  extern nsColor ConvertAssimpType(const aiColor4D& value, bool bInvert = false);
  extern nsMat4 ConvertAssimpType(const aiMatrix4x4& value, bool bDummy = false);
  extern nsVec3 ConvertAssimpType(const aiVector3D& value, bool bDummy = false);
  extern nsQuat ConvertAssimpType(const aiQuaternion& value, bool bDummy = false);
  extern float ConvertAssimpType(float value, bool bDummy = false);
  extern int ConvertAssimpType(int value, bool bDummy = false);

} // namespace nsModelImporter2
