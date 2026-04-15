// Mannequin Image Comparison Pixel Shader
// Renders a difference heatmap between two textures.
// Used by the visual test pipeline for diff visualization.

struct PSInput
{
  float4 Position : SV_POSITION;
  float2 TexCoord : TEXCOORD0;
};

Texture2D TestImage      : register(t0);
Texture2D ReferenceImage : register(t1);
SamplerState PointSampler : register(s0);

cbuffer ComparisonSettings : register(b0)
{
  float Threshold;     // Per-pixel error threshold
  float Opacity;       // Overlay opacity for blend mode
  int   ViewMode;      // 0=SideBySide, 1=Overlay, 2=DiffOnly
  float _Padding;
};

float4 main(PSInput input) : SV_TARGET
{
  float4 testColor = TestImage.Sample(PointSampler, input.TexCoord);
  float4 refColor = ReferenceImage.Sample(PointSampler, input.TexCoord);

  float3 diff = abs(testColor.rgb - refColor.rgb);
  float error = length(diff) / 1.732; // Normalize by sqrt(3)

  // Heatmap: green → yellow → red
  float3 heatmap;
  if (error < 0.5)
  {
    heatmap = lerp(float3(0, 1, 0), float3(1, 1, 0), error * 2.0);
  }
  else
  {
    heatmap = lerp(float3(1, 1, 0), float3(1, 0, 0), (error - 0.5) * 2.0);
  }

  // Highlight pixels above threshold with a bright outline
  if (error > Threshold)
    heatmap = float3(1, 0, 1); // Magenta for failed pixels

  if (ViewMode == 2)
    return float4(heatmap, 1.0);

  if (ViewMode == 1)
    return lerp(refColor, float4(heatmap, 1.0), Opacity);

  return testColor; // Side-by-side handled at viewport level
}
