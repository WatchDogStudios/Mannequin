#include <ModelImporter2/ModelImporterPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <ModelImporter2/ImporterMagicaVoxel/ImporterMagicaVoxel.h>

#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>
#include <RendererCore/Meshes/MeshResourceDescriptor.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Logging/Log.h>


#define OGT_VOX_IMPLEMENTATION
#include <ModelImporter2/ImporterMagicaVoxel/ogt_vox.h>

#define OGT_VOXEL_MESHIFY_IMPLEMENTATION
#include <ModelImporter2/ImporterMagicaVoxel/ogt_voxel_meshify.h>


namespace nsModelImporter2
{
  ImporterMagicaVoxel::ImporterMagicaVoxel() = default;
  ImporterMagicaVoxel::~ImporterMagicaVoxel() = default;

  nsResult ImporterMagicaVoxel::DoImport()
  {
    const char* szFileName = m_Options.m_sSourceFile;

    nsDynamicArray<nsUInt8> fileContent;
    fileContent.Reserve(1024 * 1024);

    // Read the whole file into memory since we map BSP data structures directly to memory content
    {
      nsFileReader fileReader;

      if (fileReader.Open(szFileName, 1024 * 1024).Failed())
      {
        nsLog::Error("Couldn't open '{}' for voxel import.", szFileName);
        return NS_FAILURE;
      }

      nsUInt8 Temp[1024 * 4];

      while (nsUInt64 uiRead = fileReader.ReadBytes(Temp, NS_ARRAY_SIZE(Temp)))
      {
        fileContent.PushBackRange(nsArrayPtr<nsUInt8>(Temp, (nsUInt32)uiRead));
      }
    }

    if (fileContent.IsEmpty())
    {
      return NS_FAILURE;
    }

    const ogt_vox_scene* scene = ogt_vox_read_scene(fileContent.GetData(), fileContent.GetCount());
    if (!scene)
    {
      nsLog::Error("Couldn't open '{}' for voxel import, read_scene failed.", szFileName);
      return NS_FAILURE;
    }

    NS_SCOPE_EXIT(ogt_vox_destroy_scene(scene));

    // Temp storage buffers to build the mesh streams out of
    nsDynamicArray<nsVec3> positions;
    positions.Reserve(4096);

    nsDynamicArray<nsVec3> normals;
    normals.Reserve(4096);

    nsDynamicArray<nsColorGammaUB> colors;
    colors.Reserve(4096);

    nsDynamicArray<nsUInt32> indices;
    indices.Reserve(8192);

    nsUInt32 uiIndexOffset = 0;

    for (uint32_t modelIdx = 0; modelIdx < scene->num_models; ++modelIdx)
    {
      const ogt_vox_model* model = scene->models[modelIdx];

      ogt_voxel_meshify_context ctx;
      memset(&ctx, 0, sizeof(ctx));

      ogt_mesh* mesh = ogt_mesh_from_paletted_voxels_greedy(&ctx, model->voxel_data, model->size_x, model->size_y, model->size_z, (const ogt_mesh_rgba*)&scene->palette.color[0]);
      NS_SCOPE_EXIT(ogt_mesh_destroy(&ctx, mesh));

      if (!mesh)
      {
        nsLog::Error("Couldn't generate mesh for voxels in file '{}'.", szFileName);
        return NS_FAILURE;
      }

      ogt_mesh_remove_duplicate_vertices(&ctx, mesh);

      // offset mesh vertices so that the center of the mesh (center of the voxel grid) is at (0,0,0)
      // also apply the root transform in the same go
      {
        const nsVec3 originOffset = nsVec3(-(float)(model->size_x >> 1), (float)(model->size_z >> 1), (float)(model->size_y >> 1));

        for (uint32_t i = 0; i < mesh->vertex_count; ++i)
        {
          nsVec3 pos = nsVec3(-mesh->vertices[i].pos.x, mesh->vertices[i].pos.z, mesh->vertices[i].pos.y);
          pos -= originOffset;
          positions.ExpandAndGetRef() = m_Options.m_RootTransform * pos;

          nsVec3 norm = nsVec3(-mesh->vertices[i].normal.x, mesh->vertices[i].normal.z, mesh->vertices[i].normal.y);
          normals.ExpandAndGetRef() = m_Options.m_RootTransform.TransformDirection(norm);

          colors.ExpandAndGetRef() = nsColorGammaUB(mesh->vertices[i].color.r, mesh->vertices[i].color.g, mesh->vertices[i].color.b, mesh->vertices[i].color.a);
        }

        for (uint32_t i = 0; i < mesh->index_count; ++i)
        {
          indices.PushBack(mesh->indices[i] + uiIndexOffset);
        }
      }

      uiIndexOffset += mesh->vertex_count;
    }


    nsMeshBufferResourceDescriptor& mb = m_Options.m_pMeshOutput->MeshBufferDesc();

    const nsUInt32 uiPosStream = mb.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    const nsUInt32 uiNrmStream = mb.AddStream(nsGALVertexAttributeSemantic::Normal, nsMeshNormalPrecision::ToResourceFormatNormal(nsMeshNormalPrecision::_10Bit));
    const nsUInt32 uiColStream = mb.AddStream(nsGALVertexAttributeSemantic::Color0, nsGALResourceFormat::RGBAUByteNormalized);

    mb.AllocateStreams(positions.GetCount(), nsGALPrimitiveTopology::Triangles, indices.GetCount() / 3);

    // Add triangles
    nsUInt32 uiFinalTriIdx = 0;
    for (nsUInt32 i = 0; i < indices.GetCount(); i += 3, ++uiFinalTriIdx)
    {
      mb.SetTriangleIndices(uiFinalTriIdx, indices[i + 1], indices[i + 0], indices[i + 2]);
    }

    for (nsUInt32 i = 0; i < positions.GetCount(); ++i)
    {
      mb.SetVertexData(uiPosStream, i, positions[i]);

      nsMeshBufferUtils::EncodeNormal(normals[i], mb.GetVertexData(uiNrmStream, i), nsMeshNormalPrecision::_10Bit).IgnoreResult();

      mb.SetVertexData(uiColStream, i, colors[i]);
    }

    m_Options.m_pMeshOutput->SetMaterial(0, nsMaterialResource::GetDefaultMaterialFileName(nsMaterialResource::DefaultMaterialType::Lit));
    m_Options.m_pMeshOutput->AddSubMesh(indices.GetCount() / 3, 0, 0);
    m_Options.m_pMeshOutput->ComputeBounds();

    return NS_SUCCESS;
  }
} // namespace nsModelImporter2
