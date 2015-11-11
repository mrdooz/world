#include "common.hlsl"

cbuffer V : register(b0)
{
  float4x4 viewProj;
}

//------------------------------------------------------
struct VSTextureIn
{
  float3 pos : Position;
  float2 uv : TexCoord;
};

struct VSTextureOut
{
  float4 pos : SV_Position;
  float2 uv : TexCoord;
};

//------------------------------------------------------
// entry-point: vs
VSTextureOut VsRenderTexture(VSTextureIn v)
{
  VSTextureOut res;
  res.pos = mul(float4(v.pos, 1), viewProj);
  res.uv = v.uv;
  return res;
}

//------------------------------------------------------
// entry-point: ps
float4 PsRenderTexture(VSTextureOut p) : SV_Target
{
  return Texture0.Sample(LinearSampler, p.uv);
}
