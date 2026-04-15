// Mannequin Default Pixel Shader
// Used by visual test pipeline for basic shaded rendering.

struct PSInput
{
  float4 Position : SV_POSITION;
  float3 Normal   : NORMAL;
  float2 TexCoord : TEXCOORD0;
  float4 Color    : COLOR0;
};

cbuffer LightData : register(b2)
{
  float3 LightDirection;
  float  LightIntensity;
  float3 LightColor;
  float  AmbientIntensity;
};

Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
  // Basic diffuse lighting
  float3 normal = normalize(input.Normal);
  float NdotL = saturate(dot(normal, -LightDirection));
  float3 diffuse = LightColor * LightIntensity * NdotL;
  float3 ambient = float3(AmbientIntensity, AmbientIntensity, AmbientIntensity);

  float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
  float3 finalColor = (ambient + diffuse) * texColor.rgb * input.Color.rgb;

  return float4(finalColor, texColor.a * input.Color.a);
}
