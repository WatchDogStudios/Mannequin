// Mannequin Fullscreen Triangle Vertex Shader
// Generates a fullscreen triangle without vertex buffers.
// Used for post-processing and image comparison overlay.

struct VSOutput
{
  float4 Position : SV_POSITION;
  float2 TexCoord : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
  VSOutput output;
  // Generate fullscreen triangle from vertex ID
  output.TexCoord = float2((vertexID << 1) & 2, vertexID & 2);
  output.Position = float4(output.TexCoord * 2.0 - 1.0, 0.0, 1.0);
  output.TexCoord.y = 1.0 - output.TexCoord.y; // Flip Y for texture sampling
  return output;
}
