#include <ModelImporter2/ModelImporterPCH.h>

#include <Foundation/Logging/Log.h>
#include <ModelImporter2/ImporterAssimp/ImporterAssimp.h>

#include <RendererCore/AnimationSystem/EditableSkeleton.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshResourceDescriptor.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace nsModelImporter2
{
  ImporterAssimp::ImporterAssimp() = default;
  ImporterAssimp::~ImporterAssimp() = default;

  class aiLogStreamError : public Assimp::LogStream
  {
  public:
    void write(const char* szMessage) { nsLog::Warning("AssImp: {0}", szMessage); }
  };

  class aiLogStreamWarning : public Assimp::LogStream
  {
  public:
    void write(const char* szMessage) { nsLog::Warning("AssImp: {0}", szMessage); }
  };

  class aiLogStreamInfo : public Assimp::LogStream
  {
  public:
    void write(const char* szMessage) { nsLog::Dev("AssImp: {0}", szMessage); }
  };

  nsResult ImporterAssimp::DoImport()
  {
    Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);

    Assimp::DefaultLogger::get()->attachStream(new aiLogStreamError(), Assimp::Logger::Err);
    Assimp::DefaultLogger::get()->attachStream(new aiLogStreamWarning(), Assimp::Logger::Warn);
    Assimp::DefaultLogger::get()->attachStream(new aiLogStreamInfo(), Assimp::Logger::Info);

    // Note: ReadFileFromMemory is not able to read dependent files even if we use our own Assimp::IOSystem.
    // It is possible to use ReadFile instead but this leads to a lot of code...
    // Triangulate:           Our mesh format cannot handle anything else.
    // JoinIdenticalVertices: Assimp doesn't use index buffer at all if this is not specified.
    // TransformUVCoords:     As of now we do not have a concept for uv transforms.
    // Process_FlipUVs:       Assimp assumes OpenGl style UV coordinate system otherwise.
    // ImproveCacheLocality:  Reorders triangles for better vertex cache locality.

    nsUInt32 uiAssimpFlags = 0;
    if (m_Options.m_pMeshOutput != nullptr)
    {
      uiAssimpFlags |= aiProcess_Triangulate | aiProcess_TransformUVCoords | aiProcess_FlipUVs | aiProcess_ImproveCacheLocality;

      if (!m_Options.m_bImportSkinningData)
      {
        // joining vertices doesn't take into account that two vertices might have different bone assignments
        // so in case of a mesh that is cut into multiple pieces (breakable object),
        // it will re-join vertices that are supposed to stay separate
        // therefore don't do this for skinned meshes
        uiAssimpFlags |= aiProcess_JoinIdenticalVertices;
      }
    }



    m_pScene = m_Importer.ReadFile(m_Options.m_sSourceFile, uiAssimpFlags);
    if (m_pScene == nullptr)
    {
      nsLog::Error("Assimp failed to import '{}'", m_Options.m_sSourceFile);
      return NS_FAILURE;
    }

    if (m_pScene->mMetaData != nullptr)
    {
      float fUnitScale = 1.0f;

      if (m_pScene->mMetaData->Get("UnitScaleFactor", fUnitScale))
      {
        // Only FBX files have this unit scale factor and the default unit for FBX is cm. We want meters.
        fUnitScale /= 100.0f;

        nsMat3 s = nsMat3::MakeScaling(nsVec3(fUnitScale));

        m_Options.m_RootTransform = s * m_Options.m_RootTransform;
      }
    }

    if (aiNode* node = m_pScene->mRootNode)
    {
      nsMat4 tmp;
      tmp.SetIdentity();
      tmp.SetRotationalPart(m_Options.m_RootTransform);
      tmp.Transpose(); // aiMatrix4x4 is row-major

      const aiMatrix4x4 transform = reinterpret_cast<aiMatrix4x4&>(tmp);

      node->mTransformation = transform * node->mTransformation;
    }

    NS_SUCCEED_OR_RETURN(ImportMaterials());

    NS_SUCCEED_OR_RETURN(TraverseAiScene());

    NS_SUCCEED_OR_RETURN(PrepareOutputMesh());

    NS_SUCCEED_OR_RETURN(ImportAnimations());

    NS_SUCCEED_OR_RETURN(ImportBoneColliders(nullptr));

    if (m_Options.m_pMeshOutput)
    {
      if (m_Options.m_bRecomputeNormals)
      {
        if (m_Options.m_pMeshOutput->MeshBufferDesc().RecomputeNormals().Failed())
        {
          nsLog::Error("Recomputing the mesh normals failed.");
          // do not return failure here, because we can still continue
        }
      }

      if (m_Options.m_bRecomputeTangents)
      {
        if (RecomputeTangents().Failed())
        {
          nsLog::Error("Recomputing the mesh tangents failed.");
          // do not return failure here, because we can still continue
        }
      }
    }

    if (m_pScene->mNumTextures > 0 && m_pScene->mTextures)
    {
      nsStringBuilder refName;

      for (nsUInt32 i = 0; i < m_pScene->mNumTextures; ++i)
      {
        const auto& st = *m_pScene->mTextures[i];

        refName.SetFormat("*{}", i);

        auto& tex = m_OutputTextures[refName];
        tex.m_sFilename = st.mFilename.C_Str();
        if (tex.m_sFilename.IsEmpty())
        {
          tex.m_sFilename = refName;
        }

        if (st.mHeight == 0 && st.mWidth > 0)
        {
          tex.m_sFileFormatExtension = st.achFormatHint;
          tex.m_RawData = nsMakeArrayPtr((const nsUInt8*)st.pcData, st.mWidth);
        }
      }
    }

    return NS_SUCCESS;
  }

  nsResult ImporterAssimp::TraverseAiScene()
  {
    if (m_Options.m_pSkeletonOutput != nullptr)
    {
      m_Options.m_pSkeletonOutput->m_Children.PushBack(NS_DEFAULT_NEW(nsEditableSkeletonJoint));
      NS_SUCCEED_OR_RETURN(TraverseAiNode(m_pScene->mRootNode, nsMat4::MakeIdentity(), m_Options.m_pSkeletonOutput->m_Children.PeekBack()));
    }
    else
    {
      NS_SUCCEED_OR_RETURN(TraverseAiNode(m_pScene->mRootNode, nsMat4::MakeIdentity(), nullptr));
    }

    return NS_SUCCESS;
  }

  nsResult ImporterAssimp::TraverseAiNode(aiNode* pNode, const nsMat4& parentTransform, nsEditableSkeletonJoint* pCurJoint)
  {
    nsMat4 invTrans = parentTransform;
    NS_ASSERT_DEBUG(invTrans.Invert(0.0f).Succeeded(), "inversion failed");

    const nsMat4 localTransform = ConvertAssimpType(pNode->mTransformation);
    const nsMat4 globalTransform = parentTransform * localTransform;

    if (pCurJoint)
    {
      pCurJoint->m_sName.Assign(pNode->mName.C_Str());
      pCurJoint->m_LocalTransform = nsTransform::MakeFromMat4(localTransform);
    }

    if (pNode->mNumMeshes > 0)
    {
      for (nsUInt32 meshIdx = 0; meshIdx < pNode->mNumMeshes; ++meshIdx)
      {
        NS_SUCCEED_OR_RETURN(ProcessAiMesh(m_pScene->mMeshes[pNode->mMeshes[meshIdx]], globalTransform));
      }
    }

    for (nsUInt32 childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
    {
      if (pCurJoint)
      {
        pCurJoint->m_Children.PushBack(NS_DEFAULT_NEW(nsEditableSkeletonJoint));

        NS_SUCCEED_OR_RETURN(TraverseAiNode(pNode->mChildren[childIdx], globalTransform, pCurJoint->m_Children.PeekBack()));
      }
      else
      {
        NS_SUCCEED_OR_RETURN(TraverseAiNode(pNode->mChildren[childIdx], globalTransform, nullptr));
      }
    }

    return NS_SUCCESS;
  }


  nsResult ImporterAssimp::ImportBoneColliders(nsEditableSkeletonJoint* pJoint)
  {
    if (m_Options.m_pSkeletonOutput == nullptr)
      return NS_SUCCESS;

    if (pJoint == nullptr)
    {
      for (nsEditableSkeletonJoint* pJoint : m_Options.m_pSkeletonOutput->m_Children)
      {
        NS_SUCCEED_OR_RETURN(ImportBoneColliders(pJoint));
      }

      return NS_SUCCESS;
    }
    else
    {
      for (nsEditableSkeletonJoint* pChild : pJoint->m_Children)
      {
        NS_SUCCEED_OR_RETURN(ImportBoneColliders(pChild));
      }
    }

    nsStringBuilder sTmp;

    const nsString& sName = pJoint->m_sName.GetString();

    for (auto meshIt : m_MeshInstances)
    {
      for (const MeshInstance& meshInst : meshIt.Value())
      {
        auto pMesh = meshInst.m_pMesh;

        if (nsStringUtils::FindSubString(pMesh->mName.C_Str(), sName) != nullptr)
        {
          sTmp = pMesh->mName.C_Str();

          if (sTmp.TrimWordStart("UCX_") && sTmp.TrimWordStart(sName) && (sTmp.IsEmpty() || sTmp.TrimWordStart("_")))
          {
            // mesh is named "UCX_BoneName_xyz" or "UCX_BoneName" -> use mesh as convex collider for this bone

            NS_ASSERT_DEV(pMesh->HasPositions(), "TODO: early out");
            NS_ASSERT_DEV(pMesh->HasFaces(), "TODO: early out");

            nsEditableSkeletonBoneCollider& col = pJoint->m_BoneColliders.ExpandAndGetRef();
            col.m_sIdentifier = pMesh->mName.C_Str();
            col.m_TriangleIndices.Reserve(pMesh->mNumFaces * 3);
            col.m_VertexPositions.Reserve(pMesh->mNumVertices);

            for (nsUInt32 v = 0; v < pMesh->mNumVertices; ++v)
            {
              col.m_VertexPositions.PushBack(meshInst.m_GlobalTransform * ConvertAssimpType(pMesh->mVertices[v]));
            }

            for (nsUInt32 f = 0; f < pMesh->mNumFaces; ++f)
            {
              col.m_TriangleIndices.PushBack(pMesh->mFaces[f].mIndices[0]);
              col.m_TriangleIndices.PushBack(pMesh->mFaces[f].mIndices[1]);
              col.m_TriangleIndices.PushBack(pMesh->mFaces[f].mIndices[2]);
            }
          }
          else
          {
            // nsLog::Error("TODO: error message");
          }
        }
      }
    }

    return NS_SUCCESS;
  }

} // namespace nsModelImporter2
