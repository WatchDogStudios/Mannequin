// Mannequin Default Vertex Shader
// Used by visual test pipeline for basic geometry rendering.

struct VSInput
{
  float3 Position : POSITION;
  float3 Normal   : NORMAL;
  float2 TexCoord : TEXCOORD0;
  float4 Color    : COLOR0;
};

struct VSOutput
{
  float4 Position : SV_POSITION;
  float3 Normal   : NORMAL;
  float2 TexCoord : TEXCOORD0;
  float4 Color    : COLOR0;
};

cbuffer PerFrame : register(b0)
{
  float4x4 ViewProjection;
};

cbuffer PerObject : register(b1)
{
  float4x4 World;
};

VSOutput main(VSInput input)
{
  VSOutput output;
  float4 worldPos = mul(World, float4(input.Position, 1.0));
  output.Position = mul(ViewProjection, worldPos);
  output.Normal = mul((float3x3)World, input.Normal);
  output.TexCoord = input.TexCoord;
  output.Color = input.Color;
  return output;
}
