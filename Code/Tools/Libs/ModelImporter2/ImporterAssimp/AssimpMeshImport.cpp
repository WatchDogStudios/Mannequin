#include <ModelImporter2/ModelImporterPCH.h>

#include <Foundation/Logging/Log.h>
#include <ModelImporter2/ImporterAssimp/ImporterAssimp.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>
#include <RendererCore/Meshes/MeshResourceDescriptor.h>
#include <assimp/scene.h>
#include <meshoptimizer/meshoptimizer.h>
#include <mikktspace/mikktspace.h>

namespace nsModelImporter2
{
  struct StreamIndices
  {
    nsUInt32 uiPositions = nsInvalidIndex;
    nsUInt32 uiNormals = nsInvalidIndex;
    nsUInt32 uiUV0 = nsInvalidIndex;
    nsUInt32 uiUV1 = nsInvalidIndex;
    nsUInt32 uiTangents = nsInvalidIndex;
    nsUInt32 uiBoneIdx = nsInvalidIndex;
    nsUInt32 uiBoneWgt = nsInvalidIndex;
    nsUInt32 uiColor0 = nsInvalidIndex;
    nsUInt32 uiColor1 = nsInvalidIndex;
  };

  void ImporterAssimp::SimplifyAiMesh(aiMesh* pMesh)
  {
    if (m_Options.m_uiMeshSimplification == 0)
      return;

    // already processed
    if (m_OptimizedMeshes.Contains(pMesh))
      return;

    m_OptimizedMeshes.Insert(pMesh);

    nsUInt32 numIndices = pMesh->mNumFaces * 3;

    nsDynamicArray<nsUInt32> indices;
    indices.Reserve(numIndices);

    nsDynamicArray<nsUInt32> simplifiedIndices;
    simplifiedIndices.SetCountUninitialized(numIndices);

    for (nsUInt32 face = 0; face < pMesh->mNumFaces; ++face)
    {
      indices.PushBack(pMesh->mFaces[face].mIndices[0]);
      indices.PushBack(pMesh->mFaces[face].mIndices[1]);
      indices.PushBack(pMesh->mFaces[face].mIndices[2]);
    }

    const float fTargetError = nsMath::Clamp<nsUInt32>(m_Options.m_uiMaxSimplificationError, 1, 99) / 100.0f;
    const size_t numTargetIndices = static_cast<size_t>((numIndices * (100 - nsMath::Min<nsUInt32>(m_Options.m_uiMeshSimplification, 99))) / 100.0f);

    float err;
    size_t numNewIndices = 0;

    if (m_Options.m_bAggressiveSimplification)
    {
      numNewIndices = meshopt_simplifySloppy(simplifiedIndices.GetData(), indices.GetData(), numIndices, &pMesh->mVertices[0].x, pMesh->mNumVertices, sizeof(aiVector3D), numTargetIndices, fTargetError, &err);
    }
    else
    {
      numNewIndices = meshopt_simplify(simplifiedIndices.GetData(), indices.GetData(), numIndices, &pMesh->mVertices[0].x, pMesh->mNumVertices, sizeof(aiVector3D), numTargetIndices, fTargetError, 0, &err);
      simplifiedIndices.SetCount(static_cast<nsUInt32>(numNewIndices));
    }

    nsDynamicArray<nsUInt32> remapTable;
    remapTable.SetCountUninitialized(pMesh->mNumVertices);
    const size_t numUniqueVerts = meshopt_optimizeVertexFetchRemap(remapTable.GetData(), simplifiedIndices.GetData(), numNewIndices, pMesh->mNumVertices);

    meshopt_remapVertexBuffer(pMesh->mVertices, pMesh->mVertices, pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());

    if (pMesh->HasNormals())
    {
      meshopt_remapVertexBuffer(pMesh->mNormals, pMesh->mNormals, pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());
    }
    if (pMesh->HasTextureCoords(0))
    {
      meshopt_remapVertexBuffer(pMesh->mTextureCoords[0], pMesh->mTextureCoords[0], pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());
    }
    if (pMesh->HasTextureCoords(1))
    {
      meshopt_remapVertexBuffer(pMesh->mTextureCoords[1], pMesh->mTextureCoords[1], pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());
    }
    if (pMesh->HasVertexColors(0))
    {
      meshopt_remapVertexBuffer(pMesh->mColors[0], pMesh->mColors[0], pMesh->mNumVertices, sizeof(aiColor4D), remapTable.GetData());
    }
    if (pMesh->HasVertexColors(1))
    {
      meshopt_remapVertexBuffer(pMesh->mColors[1], pMesh->mColors[1], pMesh->mNumVertices, sizeof(aiColor4D), remapTable.GetData());
    }
    if (pMesh->HasTangentsAndBitangents())
    {
      meshopt_remapVertexBuffer(pMesh->mTangents, pMesh->mTangents, pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());
      meshopt_remapVertexBuffer(pMesh->mBitangents, pMesh->mBitangents, pMesh->mNumVertices, sizeof(aiVector3D), remapTable.GetData());
    }
    if (pMesh->HasBones() && m_Options.m_bImportSkinningData)
    {
      for (nsUInt32 b = 0; b < pMesh->mNumBones; ++b)
      {
        auto& bone = pMesh->mBones[b];

        for (nsUInt32 w = 0; w < bone->mNumWeights;)
        {
          auto& weight = bone->mWeights[w];
          const nsUInt32 uiNewIdx = remapTable[weight.mVertexId];

          if (uiNewIdx == ~0u)
          {
            // this vertex got removed -> swap it with the last weight
            bone->mWeights[w] = bone->mWeights[bone->mNumWeights - 1];
            --bone->mNumWeights;
          }
          else
          {
            bone->mWeights[w].mVertexId = uiNewIdx;
            ++w;
          }
        }
      }
    }

    pMesh->mNumVertices = static_cast<nsUInt32>(numUniqueVerts);

    nsDynamicArray<nsUInt32> newIndices;
    newIndices.SetCountUninitialized(static_cast<nsUInt32>(numNewIndices));

    meshopt_remapIndexBuffer(newIndices.GetData(), simplifiedIndices.GetData(), numNewIndices, remapTable.GetData());

    pMesh->mNumFaces = static_cast<nsUInt32>(numNewIndices / 3);

    nsUInt32 nextIdx = 0;
    for (nsUInt32 face = 0; face < pMesh->mNumFaces; ++face)
    {
      pMesh->mFaces[face].mIndices[0] = newIndices[nextIdx++];
      pMesh->mFaces[face].mIndices[1] = newIndices[nextIdx++];
      pMesh->mFaces[face].mIndices[2] = newIndices[nextIdx++];
    }
  }

  nsResult ImporterAssimp::ProcessAiMesh(aiMesh* pMesh, const nsMat4& transform)
  {
    if ((pMesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_TRIANGLE) == 0) // no triangles in there ?
      return NS_SUCCESS;

    if (m_Options.m_bImportSkinningData && !pMesh->HasBones())
    {
      nsLog::Warning("Mesh contains an unskinned part ('{}' - {} triangles)", pMesh->mName.C_Str(), pMesh->mNumFaces);
      return NS_SUCCESS;
    }

    // if enabled, the aiMesh is modified in-place to have less detail
    SimplifyAiMesh(pMesh);

    {
      auto& mi = m_MeshInstances[pMesh->mMaterialIndex].ExpandAndGetRef();
      mi.m_GlobalTransform = transform;
      mi.m_pMesh = pMesh;

      m_uiTotalMeshVertices += pMesh->mNumVertices;
      m_uiTotalMeshTriangles += pMesh->mNumFaces;
    }

    return NS_SUCCESS;
  }

  static void SetMeshTriangleIndices(nsMeshBufferResourceDescriptor& ref_mb, const aiMesh* pMesh, nsUInt32 uiTriangleIndexOffset, nsUInt32 uiVertexIndexOffset, bool bFlipTriangles)
  {
    if (bFlipTriangles)
    {
      for (nsUInt32 triIdx = 0; triIdx < pMesh->mNumFaces; ++triIdx)
      {
        const nsUInt32 finalTriIdx = uiTriangleIndexOffset + triIdx;

        const nsUInt32 f0 = pMesh->mFaces[triIdx].mIndices[0];
        const nsUInt32 f1 = pMesh->mFaces[triIdx].mIndices[1];
        const nsUInt32 f2 = pMesh->mFaces[triIdx].mIndices[2];

        ref_mb.SetTriangleIndices(finalTriIdx, uiVertexIndexOffset + f0, uiVertexIndexOffset + f2, uiVertexIndexOffset + f1);
      }
    }
    else
    {
      for (nsUInt32 triIdx = 0; triIdx < pMesh->mNumFaces; ++triIdx)
      {
        const nsUInt32 finalTriIdx = uiTriangleIndexOffset + triIdx;

        const nsUInt32 f0 = pMesh->mFaces[triIdx].mIndices[0];
        const nsUInt32 f1 = pMesh->mFaces[triIdx].mIndices[1];
        const nsUInt32 f2 = pMesh->mFaces[triIdx].mIndices[2];

        ref_mb.SetTriangleIndices(finalTriIdx, uiVertexIndexOffset + f0, uiVertexIndexOffset + f1, uiVertexIndexOffset + f2);
      }
    }
  }

  static void SetMeshBoneData(nsMeshBufferResourceDescriptor& ref_mb, nsMeshResourceDescriptor& ref_mrd, float& inout_fMaxBoneOffset, const aiMesh* pMesh, nsUInt32 uiVertexIndexOffset, const StreamIndices& streams, bool b8BitBoneIndices, nsMeshBoneWeigthPrecision::Enum weightsPrecision, bool bNormalizeWeights)
  {
    if (!pMesh->HasBones())
      return;

    nsHashedString hs;

    for (nsUInt32 b = 0; b < pMesh->mNumBones; ++b)
    {
      const aiBone* pBone = pMesh->mBones[b];

      hs.Assign(pBone->mName.C_Str());
      const nsUInt32 uiBoneIndex = ref_mrd.m_Bones[hs].m_uiBoneIndex;

      for (nsUInt32 w = 0; w < pBone->mNumWeights; ++w)
      {
        const auto& weight = pBone->mWeights[w];

        const nsUInt32 finalVertIdx = uiVertexIndexOffset + weight.mVertexId;

        nsUInt8* pBoneIndices8 = reinterpret_cast<nsUInt8*>(ref_mb.GetVertexData(streams.uiBoneIdx, finalVertIdx).GetPtr());
        nsUInt16* pBoneIndices16 = reinterpret_cast<nsUInt16*>(pBoneIndices8);
        nsByteArrayPtr pBoneWeights = ref_mb.GetVertexData(streams.uiBoneWgt, finalVertIdx);

        nsVec4 wgt;
        nsMeshBufferUtils::DecodeToVec4(pBoneWeights, nsMeshBoneWeigthPrecision::ToResourceFormat(weightsPrecision), wgt).AssertSuccess();

        // pBoneWeights are initialized with 0
        // so for the first 4 bones we always assign to one slot that is 0 (least weight)
        // if we have 5 bones or more, we then replace the currently lowest value each time

        nsUInt32 uiLeastWeightIdx = 0;

        for (int i = 1; i < 4; ++i)
        {
          if (wgt.GetData()[i] < wgt.GetData()[uiLeastWeightIdx])
          {
            uiLeastWeightIdx = i;
          }
        }

        const float fEncodedWeight = weight.mWeight;

        if (wgt.GetData()[uiLeastWeightIdx] < fEncodedWeight)
        {
          wgt.GetData()[uiLeastWeightIdx] = fEncodedWeight;

          nsMeshBufferUtils::EncodeBoneWeights(wgt, pBoneWeights, nsMeshBoneWeigthPrecision::ToResourceFormat(weightsPrecision)).AssertSuccess();

          if (b8BitBoneIndices)
            pBoneIndices8[uiLeastWeightIdx] = static_cast<nsUInt8>(uiBoneIndex);
          else
            pBoneIndices16[uiLeastWeightIdx] = static_cast<nsUInt16>(uiBoneIndex);
        }
      }
    }

    for (nsUInt32 vtx = 0; vtx < ref_mb.GetVertexCount(); ++vtx)
    {
      nsByteArrayPtr pBoneWeights = ref_mb.GetVertexData(streams.uiBoneWgt, vtx);

      nsVec4 wgt;
      nsMeshBufferUtils::DecodeToVec4(pBoneWeights, nsMeshBoneWeigthPrecision::ToResourceFormat(weightsPrecision), wgt).AssertSuccess();

      // normalize the bone weights
      if (bNormalizeWeights)
      {
        // NOTE: This is absolutely crucial for some meshes to work right
        // On the other hand, it is also possible that some meshes don't like this
        // if we come across meshes where normalization breaks them, we may need to add a user-option to select whether bone weights should be normalized

        const float len = wgt.x + wgt.y + wgt.z + wgt.w;
        if (len > 1.0f)
        {
          wgt /= len;

          nsMeshBufferUtils::EncodeBoneWeights(wgt, pBoneWeights, weightsPrecision).AssertSuccess();
        }
      }

      const nsVec3 vVertexPos = *reinterpret_cast<const nsVec3*>(ref_mb.GetVertexData(streams.uiPositions, vtx).GetPtr());
      const nsUInt8* pBoneIndices8 = reinterpret_cast<const nsUInt8*>(ref_mb.GetVertexData(streams.uiBoneIdx, vtx).GetPtr());
      const nsUInt16* pBoneIndices16 = reinterpret_cast<const nsUInt16*>(ref_mb.GetVertexData(streams.uiBoneIdx, vtx).GetPtr());

      // also find the maximum distance of any vertex to its influencing bones
      // this is used to adjust the bounding box for culling at runtime
      // ie. we can compute the bounding box from a pose, but that only covers the skeleton, not the full mesh
      // so we then grow the bbox by this maximum distance
      // that usually creates a far larger bbox than necessary, but means there are no culling artifacts

      nsUInt16 uiBoneId[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

      if (b8BitBoneIndices)
      {
        uiBoneId[0] = pBoneIndices8[0];
        uiBoneId[1] = pBoneIndices8[1];
        uiBoneId[2] = pBoneIndices8[2];
        uiBoneId[3] = pBoneIndices8[3];
      }
      else
      {
        uiBoneId[0] = pBoneIndices16[0];
        uiBoneId[1] = pBoneIndices16[1];
        uiBoneId[2] = pBoneIndices16[2];
        uiBoneId[3] = pBoneIndices16[3];
      }

      for (const auto& bone : ref_mrd.m_Bones)
      {
        for (int b = 0; b < 4; ++b)
        {
          if (wgt.GetData()[b] < 0.2f) // only look at bones that have a proper weight
            continue;

          if (bone.Value().m_uiBoneIndex == uiBoneId[b])
          {
            // move the vertex into local space of the bone, then determine how far it is away from the bone
            const nsVec3 vOffPos = bone.Value().m_GlobalInverseRestPoseMatrix * vVertexPos;
            const float length = vOffPos.GetLength();

            if (length > inout_fMaxBoneOffset)
            {
              inout_fMaxBoneOffset = length;
            }
          }
        }
      }
    }
  }

  static void SetMeshVertexData(nsMeshBufferResourceDescriptor& ref_mb, const aiMesh* pMesh, const nsMat4& mGlobalTransform, nsUInt32 uiVertexIndexOffset, const StreamIndices& streams, nsEnum<nsMeshNormalPrecision> meshNormalsPrecision, nsEnum<nsMeshTexCoordPrecision> meshTexCoordsPrecision, nsEnum<nsMeshVertexColorConversion> meshVertexColorConversion)
  {
    nsMat3 normalsTransform = mGlobalTransform.GetRotationalPart();
    if (normalsTransform.Invert(0.0f).Failed())
    {
      nsLog::Warning("Couldn't invert a mesh's transform matrix.");
      normalsTransform.SetIdentity();
    }

    normalsTransform.Transpose();

    for (nsUInt32 vertIdx = 0; vertIdx < pMesh->mNumVertices; ++vertIdx)
    {
      const nsUInt32 finalVertIdx = uiVertexIndexOffset + vertIdx;

      const nsVec3 position = mGlobalTransform * ConvertAssimpType(pMesh->mVertices[vertIdx]);
      ref_mb.SetVertexData(streams.uiPositions, finalVertIdx, position);

      if (streams.uiNormals != nsInvalidIndex && pMesh->HasNormals())
      {
        nsVec3 normal = normalsTransform * ConvertAssimpType(pMesh->mNormals[vertIdx]);
        normal.NormalizeIfNotZero(nsVec3::MakeZero()).IgnoreResult();

        nsMeshBufferUtils::EncodeNormal(normal, ref_mb.GetVertexData(streams.uiNormals, finalVertIdx), meshNormalsPrecision).IgnoreResult();
      }

      if (streams.uiUV0 != nsInvalidIndex && pMesh->HasTextureCoords(0))
      {
        const nsVec2 texcoord = ConvertAssimpType(pMesh->mTextureCoords[0][vertIdx]).GetAsVec2();

        nsMeshBufferUtils::EncodeTexCoord(texcoord, ref_mb.GetVertexData(streams.uiUV0, finalVertIdx), meshTexCoordsPrecision).IgnoreResult();
      }

      if (streams.uiUV1 != nsInvalidIndex && pMesh->HasTextureCoords(1))
      {
        const nsVec2 texcoord = ConvertAssimpType(pMesh->mTextureCoords[1][vertIdx]).GetAsVec2();

        nsMeshBufferUtils::EncodeTexCoord(texcoord, ref_mb.GetVertexData(streams.uiUV1, finalVertIdx), meshTexCoordsPrecision).IgnoreResult();
      }

      if (streams.uiColor0 != nsInvalidIndex && pMesh->HasVertexColors(0))
      {
        const nsColor color = ConvertAssimpType(pMesh->mColors[0][vertIdx]);

        nsMeshBufferUtils::EncodeColor(color.GetAsVec4(), ref_mb.GetVertexData(streams.uiColor0, finalVertIdx), meshVertexColorConversion).IgnoreResult();
      }

      if (streams.uiColor1 != nsInvalidIndex && pMesh->HasVertexColors(1))
      {
        const nsColor color = ConvertAssimpType(pMesh->mColors[1][vertIdx]);

        nsMeshBufferUtils::EncodeColor(color.GetAsVec4(), ref_mb.GetVertexData(streams.uiColor1, finalVertIdx), meshVertexColorConversion).IgnoreResult();
      }

      if (streams.uiTangents != nsInvalidIndex && pMesh->HasTangentsAndBitangents())
      {
        nsVec3 normal = normalsTransform * ConvertAssimpType(pMesh->mNormals[vertIdx]);
        nsVec3 tangent = normalsTransform * ConvertAssimpType(pMesh->mTangents[vertIdx]);
        nsVec3 bitangent = normalsTransform * ConvertAssimpType(pMesh->mBitangents[vertIdx]);

        normal.NormalizeIfNotZero(nsVec3::MakeZero()).IgnoreResult();
        tangent.NormalizeIfNotZero(nsVec3::MakeZero()).IgnoreResult();
        bitangent.NormalizeIfNotZero(nsVec3::MakeZero()).IgnoreResult();

        const float fBitangentSign = nsMath::Abs(tangent.CrossRH(bitangent).Dot(normal));

        nsMeshBufferUtils::EncodeTangent(tangent, fBitangentSign, ref_mb.GetVertexData(streams.uiTangents, finalVertIdx), meshNormalsPrecision).IgnoreResult();
      }
    }
  }

  static void AllocateMeshStreams(nsMeshBufferResourceDescriptor& ref_mb, nsArrayPtr<aiMesh*> referenceMeshes, StreamIndices& inout_streams, nsUInt32 uiTotalMeshVertices, nsUInt32 uiTotalMeshTriangles, nsEnum<nsMeshNormalPrecision> meshNormalsPrecision, nsEnum<nsMeshTexCoordPrecision> meshTexCoordsPrecision,
    bool bImportSkinningData, bool b8BitBoneIndices, nsEnum<nsMeshBoneWeigthPrecision> meshWeightsPrecision)
  {
    inout_streams.uiPositions = ref_mb.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    inout_streams.uiNormals = ref_mb.AddStream(nsGALVertexAttributeSemantic::Normal, nsMeshNormalPrecision::ToResourceFormatNormal(meshNormalsPrecision));
    inout_streams.uiUV0 = ref_mb.AddStream(nsGALVertexAttributeSemantic::TexCoord0, nsMeshTexCoordPrecision::ToResourceFormat(meshTexCoordsPrecision));
    inout_streams.uiTangents = ref_mb.AddStream(nsGALVertexAttributeSemantic::Tangent, nsMeshNormalPrecision::ToResourceFormatTangent(meshNormalsPrecision));

    if (bImportSkinningData)
    {
      if (b8BitBoneIndices)
        inout_streams.uiBoneIdx = ref_mb.AddStream(nsGALVertexAttributeSemantic::BoneIndices0, nsGALResourceFormat::RGBAUByte);
      else
        inout_streams.uiBoneIdx = ref_mb.AddStream(nsGALVertexAttributeSemantic::BoneIndices0, nsGALResourceFormat::RGBAUShort);

      inout_streams.uiBoneWgt = ref_mb.AddStream(nsGALVertexAttributeSemantic::BoneWeights0, nsMeshBoneWeigthPrecision::ToResourceFormat(meshWeightsPrecision));
    }

    bool bTexCoords1 = false;
    bool bVertexColors0 = false;
    bool bVertexColors1 = false;

    for (auto pMesh : referenceMeshes)
    {
      if (pMesh->HasTextureCoords(1))
        bTexCoords1 = true;
      if (pMesh->HasVertexColors(0))
        bVertexColors0 = true;
      if (pMesh->HasVertexColors(1))
        bVertexColors1 = true;
    }

    if (bTexCoords1)
    {
      inout_streams.uiUV1 = ref_mb.AddStream(nsGALVertexAttributeSemantic::TexCoord1, nsMeshTexCoordPrecision::ToResourceFormat(meshTexCoordsPrecision));
    }

    if (bVertexColors0)
    {
      inout_streams.uiColor0 = ref_mb.AddStream(nsGALVertexAttributeSemantic::Color0, nsGALResourceFormat::RGBAUByteNormalized);
    }
    if (bVertexColors1)
    {
      inout_streams.uiColor1 = ref_mb.AddStream(nsGALVertexAttributeSemantic::Color1, nsGALResourceFormat::RGBAUByteNormalized);
    }

    ref_mb.AllocateStreams(uiTotalMeshVertices, nsGALPrimitiveTopology::Triangles, uiTotalMeshTriangles, true);
  }

  static void SetMeshBindPoseData(nsMeshResourceDescriptor& ref_mrd, const aiMesh* pMesh, const nsMat4& mGlobalTransform)
  {
    if (!pMesh->HasBones())
      return;

    nsHashedString hs;

    for (nsUInt32 b = 0; b < pMesh->mNumBones; ++b)
    {
      auto pBone = pMesh->mBones[b];

      auto invPose = ConvertAssimpType(pBone->mOffsetMatrix);
      NS_VERIFY(invPose.Invert(0.0f).Succeeded(), "Inverting the bind pose matrix failed");
      invPose = mGlobalTransform * invPose;
      NS_VERIFY(invPose.Invert(0.0f).Succeeded(), "Inverting the bind pose matrix failed");

      hs.Assign(pBone->mName.C_Str());
      ref_mrd.m_Bones[hs].m_GlobalInverseRestPoseMatrix = invPose;
    }
  }

  struct MikkData
  {
    nsMeshBufferResourceDescriptor* m_pMeshBuffer = nullptr;
    nsUInt32 m_uiVertexSize = 0;
    const nsUInt16* m_pIndices16 = nullptr;
    const nsUInt32* m_pIndices32 = nullptr;
    const nsUInt8* m_pPositions = nullptr;
    const nsUInt8* m_pNormals = nullptr;
    const nsUInt8* m_pTexCoords = nullptr;
    nsUInt8* m_pTangents = nullptr;
    nsGALResourceFormat::Enum m_NormalsFormat;
    nsGALResourceFormat::Enum m_TexCoordsFormat;
    nsGALResourceFormat::Enum m_TangentsFormat;
  };

  static int MikkGetNumFaces(const SMikkTSpaceContext* pContext)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    return pMikkData->m_pMeshBuffer->GetPrimitiveCount();
  }

  static int MikkGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, int iFace)
  { //
    return 3;
  }

  static void MikkGetPosition16(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices16[iFace * 3 + iVert];

    const nsVec3* pSrcData = reinterpret_cast<const nsVec3*>(pMikkData->m_pPositions + (uiVertexIdx * pMikkData->m_uiVertexSize));
    pData[0] = pSrcData->x;
    pData[1] = pSrcData->y;
    pData[2] = pSrcData->z;
  }

  static void MikkGetPosition32(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);

    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices32[iFace * 3 + iVert];

    const nsVec3* pSrcData = reinterpret_cast<const nsVec3*>(pMikkData->m_pPositions + (uiVertexIdx * pMikkData->m_uiVertexSize));
    pData[0] = pSrcData->x;
    pData[1] = pSrcData->y;
    pData[2] = pSrcData->z;
  }

  static void MikkGetNormal16(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices16[iFace * 3 + iVert];

    nsVec3* pDest = reinterpret_cast<nsVec3*>(pData);
    nsMeshBufferUtils::DecodeNormal(nsConstByteArrayPtr(pMikkData->m_pNormals + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_NormalsFormat, *pDest).IgnoreResult();
  }

  static void MikkGetNormal32(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices32[iFace * 3 + iVert];

    nsVec3* pDest = reinterpret_cast<nsVec3*>(pData);
    nsMeshBufferUtils::DecodeNormal(nsConstByteArrayPtr(pMikkData->m_pNormals + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_NormalsFormat, *pDest).IgnoreResult();
  }

  static void MikkGetTexCoord16(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices16[iFace * 3 + iVert];

    nsVec2* pDest = reinterpret_cast<nsVec2*>(pData);
    nsMeshBufferUtils::DecodeTexCoord(nsConstByteArrayPtr(pMikkData->m_pTexCoords + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_TexCoordsFormat, *pDest).IgnoreResult();
  }

  static void MikkGetTexCoord32(const SMikkTSpaceContext* pContext, float pData[], int iFace, int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices32[iFace * 3 + iVert];

    nsVec2* pDest = reinterpret_cast<nsVec2*>(pData);
    nsMeshBufferUtils::DecodeTexCoord(nsConstByteArrayPtr(pMikkData->m_pTexCoords + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_TexCoordsFormat, *pDest).IgnoreResult();
  }

  static void MikkSetTangents16(const SMikkTSpaceContext* pContext, const float pTangent[], const float fSign, const int iFace, const int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices16[iFace * 3 + iVert];

    const nsVec3 tangent = *reinterpret_cast<const nsVec3*>(pTangent);

    nsMeshBufferUtils::EncodeTangent(tangent, fSign, nsByteArrayPtr(pMikkData->m_pTangents + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_TangentsFormat).IgnoreResult();
  }

  static void MikkSetTangents32(const SMikkTSpaceContext* pContext, const float pTangent[], const float fSign, const int iFace, const int iVert)
  {
    MikkData* pMikkData = static_cast<MikkData*>(pContext->m_pUserData);
    const nsUInt32 uiVertexIdx = pMikkData->m_pIndices32[iFace * 3 + iVert];

    const nsVec3 tangent = *reinterpret_cast<const nsVec3*>(pTangent);

    nsMeshBufferUtils::EncodeTangent(tangent, fSign, nsByteArrayPtr(pMikkData->m_pTangents + (uiVertexIdx * pMikkData->m_uiVertexSize), 32), pMikkData->m_TangentsFormat).IgnoreResult();
  }

  nsResult ImporterAssimp::RecomputeTangents()
  {
    auto& md = m_Options.m_pMeshOutput->MeshBufferDesc();

    if (!md.HasIndexBuffer())
      return NS_FAILURE;

    MikkData mikkd;
    mikkd.m_pMeshBuffer = &md;
    mikkd.m_uiVertexSize = md.GetVertexDataSize();
    mikkd.m_pIndices16 = reinterpret_cast<const nsUInt16*>(md.GetIndexBufferData().GetData());
    mikkd.m_pIndices32 = reinterpret_cast<const nsUInt32*>(md.GetIndexBufferData().GetData());

    for (nsUInt32 i = 0; i < md.GetVertexDeclaration().m_VertexStreams.GetCount(); ++i)
    {
      const auto& stream = md.GetVertexDeclaration().m_VertexStreams[i];

      if (stream.m_Semantic == nsGALVertexAttributeSemantic::Position)
      {
        mikkd.m_pPositions = md.GetVertexData(i, 0).GetPtr();
      }
      else if (stream.m_Semantic == nsGALVertexAttributeSemantic::Normal)
      {
        mikkd.m_pNormals = md.GetVertexData(i, 0).GetPtr();
        mikkd.m_NormalsFormat = stream.m_Format;
      }
      else if (stream.m_Semantic == nsGALVertexAttributeSemantic::TexCoord0)
      {
        mikkd.m_pTexCoords = md.GetVertexData(i, 0).GetPtr();
        mikkd.m_TexCoordsFormat = stream.m_Format;
      }
      else if (stream.m_Semantic == nsGALVertexAttributeSemantic::Tangent)
      {
        mikkd.m_pTangents = md.GetVertexData(i, 0).GetPtr();
        mikkd.m_TangentsFormat = stream.m_Format;
      }
    }

    if (mikkd.m_pPositions == nullptr || mikkd.m_pTexCoords == nullptr || mikkd.m_pNormals == nullptr || mikkd.m_pTangents == nullptr)
      return NS_FAILURE;

    // Use Morton S. Mikkelsen's tangent calculation.
    SMikkTSpaceContext context;
    SMikkTSpaceInterface functions;
    context.m_pUserData = &mikkd;
    context.m_pInterface = &functions;

    functions.m_setTSpace = nullptr;
    functions.m_getNumFaces = MikkGetNumFaces;
    functions.m_getNumVerticesOfFace = MikkGetNumVerticesOfFace;

    if (md.Uses32BitIndices())
    {
      functions.m_getPosition = MikkGetPosition32;
      functions.m_getNormal = MikkGetNormal32;
      functions.m_getTexCoord = MikkGetTexCoord32;
      functions.m_setTSpaceBasic = MikkSetTangents32;
    }
    else
    {
      functions.m_getPosition = MikkGetPosition16;
      functions.m_getNormal = MikkGetNormal16;
      functions.m_getTexCoord = MikkGetTexCoord16;
      functions.m_setTSpaceBasic = MikkSetTangents16;
    }

    if (!genTangSpaceDefault(&context))
      return NS_FAILURE;

    return NS_SUCCESS;
  }

  nsResult ImporterAssimp::PrepareOutputMesh()
  {
    if (m_Options.m_pMeshOutput == nullptr)
      return NS_SUCCESS;

    auto& mb = m_Options.m_pMeshOutput->MeshBufferDesc();

    if (m_Options.m_bImportSkinningData)
    {
      for (auto itMesh : m_MeshInstances)
      {
        for (const auto& mi : itMesh.Value())
        {
          SetMeshBindPoseData(*m_Options.m_pMeshOutput, mi.m_pMesh, mi.m_GlobalTransform);
        }
      }

      nsUInt16 uiBoneCounter = 0;
      for (auto itBone : m_Options.m_pMeshOutput->m_Bones)
      {
        itBone.Value().m_uiBoneIndex = uiBoneCounter;
        ++uiBoneCounter;
      }
    }

    const bool b8BitBoneIndices = m_Options.m_pMeshOutput->m_Bones.GetCount() <= 255;

    StreamIndices streams;
    AllocateMeshStreams(mb, nsArrayPtr<aiMesh*>(m_pScene->mMeshes, m_pScene->mNumMeshes), streams, m_uiTotalMeshVertices, m_uiTotalMeshTriangles, m_Options.m_MeshNormalsPrecision, m_Options.m_MeshTexCoordsPrecision, m_Options.m_bImportSkinningData, b8BitBoneIndices, m_Options.m_MeshBoneWeightPrecision);

    nsUInt32 uiMeshPrevTriangleIdx = 0;
    nsUInt32 uiMeshCurVertexIdx = 0;
    nsUInt32 uiMeshCurTriangleIdx = 0;
    nsUInt32 uiMeshCurSubmeshIdx = 0;

    const bool bFlipTriangles = nsGraphicsUtils::IsTriangleFlipRequired(m_Options.m_RootTransform);

    for (auto itMesh : m_MeshInstances)
    {
      const nsUInt32 uiMaterialIdx = itMesh.Key();

      for (const auto& mi : itMesh.Value())
      {
        if (m_Options.m_bImportSkinningData && !mi.m_pMesh->HasBones())
        {
          // skip meshes that have no bones
          continue;
        }

        SetMeshVertexData(mb, mi.m_pMesh, mi.m_GlobalTransform, uiMeshCurVertexIdx, streams, m_Options.m_MeshNormalsPrecision, m_Options.m_MeshTexCoordsPrecision, m_Options.m_MeshVertexColorConversion);

        if (m_Options.m_bImportSkinningData)
        {
          SetMeshBoneData(mb, *m_Options.m_pMeshOutput, m_Options.m_pMeshOutput->m_fMaxBoneVertexOffset, mi.m_pMesh, uiMeshCurVertexIdx, streams, b8BitBoneIndices, m_Options.m_MeshBoneWeightPrecision, m_Options.m_bNormalizeWeights);
        }

        SetMeshTriangleIndices(mb, mi.m_pMesh, uiMeshCurTriangleIdx, uiMeshCurVertexIdx, bFlipTriangles);

        uiMeshCurTriangleIdx += mi.m_pMesh->mNumFaces;
        uiMeshCurVertexIdx += mi.m_pMesh->mNumVertices;
      }

      if (uiMeshCurTriangleIdx - uiMeshPrevTriangleIdx == 0)
      {
        // skip empty submeshes
        continue;
      }

      if (uiMaterialIdx >= m_OutputMaterials.GetCount())
      {
        m_Options.m_pMeshOutput->SetMaterial(uiMeshCurSubmeshIdx, "");
      }
      else
      {
        m_OutputMaterials[uiMaterialIdx].m_iReferencedByMesh = static_cast<nsInt32>(uiMeshCurSubmeshIdx);
        m_Options.m_pMeshOutput->SetMaterial(uiMeshCurSubmeshIdx, m_OutputMaterials[uiMaterialIdx].m_sName);
      }

      m_Options.m_pMeshOutput->AddSubMesh(uiMeshCurTriangleIdx - uiMeshPrevTriangleIdx, uiMeshPrevTriangleIdx, uiMeshCurSubmeshIdx);

      uiMeshPrevTriangleIdx = uiMeshCurTriangleIdx;
      ++uiMeshCurSubmeshIdx;
    }

    m_Options.m_pMeshOutput->ComputeBounds();

    return NS_SUCCESS;
  }
} // namespace nsModelImporter2
