SamplerState TrilinearWrap : register(s0);

Texture2D RainTexture : register(t0);

struct PixelInputType
{
	float4 ProjectedPosition : SV_POSITION;
	float2 UV				 : TEXCOORD0;
};

float4 RainPS(PixelInputType input) : SV_TARGET
{
	return RainTexture.Sample(TrilinearWrap, float3(input.UV, 0));
}