cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
};

cbuffer WaveBuffer : register(b1)
{
	float WaveScale;
	float3 waveScalePadding;
};

struct VertexInputType
{
	float3 position : POSITION;
	float3 Normal : NORMAL;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 ProjPos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 tex : TEXCOORD0;
};

static const float HeightMapHeightOverWidth = 1 / 32.0f;
static const float WaterWidth = 400.0f; 
static const float MaxWaveHeight = WaterWidth * HeightMapHeightOverWidth;

// Vertex shader
PixelInputType WaterVS(VertexInputType input)
{
	PixelInputType output;

	float4 modelPos = float4(input.position, 1.0f);

	// Sample the height of the normal map at this point in order to get the correct height of this wave.
	float height = 2.0f;

	modelPos.y += (0.25f * height - 0.5f) * MaxWaveHeight * WaveScale;

	float4 worldPosition = mul(modelPos, worldMatrix);
	output.WorldPos = worldPosition.xyz;

	float4 viewPosition = mul(worldPosition, viewMatrix);
	output.ProjPos = mul(viewPosition, projMatrix);

	output.tex = input.tex;
	return output;
}