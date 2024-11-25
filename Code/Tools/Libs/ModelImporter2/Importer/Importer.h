#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <ModelImporter2/ModelImporterDLL.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>

class nsLogInterface;
class nsProgress;
class nsEditableSkeleton;
class nsMeshResourceDescriptor;
struct nsAnimationClipResourceDescriptor;

namespace nsModelImporter2
{
  struct ImportOptions
  {
    nsString m_sSourceFile;

    bool m_bImportSkinningData = false;
    bool m_bRecomputeNormals = false;
    bool m_bRecomputeTangents = false;
    bool m_bNormalizeWeights = false;
    nsMat3 m_RootTransform = nsMat3::MakeIdentity();

    nsMeshResourceDescriptor* m_pMeshOutput = nullptr;
    nsEnum<nsMeshNormalPrecision> m_MeshNormalsPrecision = nsMeshNormalPrecision::Default;
    nsEnum<nsMeshTexCoordPrecision> m_MeshTexCoordsPrecision = nsMeshTexCoordPrecision::Default;
    nsEnum<nsMeshBoneWeigthPrecision> m_MeshBoneWeightPrecision = nsMeshBoneWeigthPrecision::Default;
    nsEnum<nsMeshVertexColorConversion> m_MeshVertexColorConversion = nsMeshVertexColorConversion::Default;

    nsEditableSkeleton* m_pSkeletonOutput = nullptr;

    bool m_bAdditiveAnimation = false;
    nsString m_sAnimationToImport; // empty = first in file; "name" = only anim with that name
    nsAnimationClipResourceDescriptor* m_pAnimationOutput = nullptr;
    nsUInt32 m_uiFirstAnimKeyframe = 0;
    nsUInt32 m_uiNumAnimKeyframes = 0;

    nsUInt8 m_uiMeshSimplification = 0;
    nsUInt8 m_uiMaxSimplificationError = 5;
    bool m_bAggressiveSimplification = false;
  };

  enum class PropertySemantic : nsInt8
  {
    Unknown = 0,

    DiffuseColor,
    RoughnessValue,
    MetallicValue,
    EmissiveColor,
    TwosidedValue,
  };

  enum class TextureSemantic : nsInt8
  {
    Unknown = 0,

    DiffuseMap,
    DiffuseAlphaMap,
    OcclusionMap,
    RoughnessMap,
    MetallicMap,
    OrmMap,
    DisplacementMap,
    NormalMap,
    EmissiveMap,
  };

  struct NS_MODELIMPORTER2_DLL OutputTexture
  {
    nsString m_sFilename;
    nsString m_sFileFormatExtension;
    nsConstByteArrayPtr m_RawData;

    void GenerateFileName(nsStringBuilder& out_sName) const;
  };

  struct NS_MODELIMPORTER2_DLL OutputMaterial
  {
    nsString m_sName;

    nsInt32 m_iReferencedByMesh = -1;                     // if -1, no sub-mesh in the output actually references this
    nsMap<TextureSemantic, nsString> m_TextureReferences; // semantic -> path
    nsMap<PropertySemantic, nsVariant> m_Properties;      // semantic -> value
  };

  class NS_MODELIMPORTER2_DLL Importer
  {
  public:
    Importer();
    virtual ~Importer();

    nsResult Import(const ImportOptions& options, nsLogInterface* pLogInterface = nullptr, nsProgress* pProgress = nullptr);
    const ImportOptions& GetImportOptions() const { return m_Options; }

    nsMap<nsString, OutputTexture> m_OutputTextures; // path -> additional data
    nsDeque<OutputMaterial> m_OutputMaterials;
    nsDynamicArray<nsString> m_OutputAnimationNames;

  protected:
    virtual nsResult DoImport() = 0;

    ImportOptions m_Options;
    nsProgress* m_pProgress = nullptr;
  };

} // namespace nsModelImporter2
