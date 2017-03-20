//////////////////////////
// Sample states
//////////////////////////

SamplerState TrilinearWrap : register(s0);

//////////////////////////
// Textures
//////////////////////////

Texture2D NormalHeightMap : register(t0);

//////////////////////////
// Constant buffers
//////////////////////////

cbuffer MatrixBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
}

cbuffer WaterBuffer : register(b1)
{
	float4 WaterSize;
	float4 WaterSpeed;
	float2 WaterTranslation;
	float WaveHeight;
	float WaveScale;
	float RefractionDistortion;
	float ReflectionDistortion;
	float MaxDistortionDistance;
	float RefractionStrength;
	float ReflectionStrength;
}

//////////////////////////
// Structures
//////////////////////////

struct VertexInputType
{
	float4 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
};

struct PixelInputType
{
	float4 ProjectedPosition : SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 WorldPosition : POSITION;
};

//////////////////////////
// Vertex shader
//////////////////////////

PixelInputType WaterSurfaceVS(VertexInputType input)
{
	PixelInputType output;

	float2 waterUV = input.UV;
	float normal1 = NormalHeightMap.SampleLevel(TrilinearWrap, WaterSize.w * (waterUV + WaterTranslation * WaterSpeed.w), 0.0f).a;
	float normal2 = NormalHeightMap.SampleLevel(TrilinearWrap, WaterSize.x * (waterUV + WaterTranslation * WaterSpeed.x), 0.0f).a;
	float normal3 = NormalHeightMap.SampleLevel(TrilinearWrap, WaterSize.y * (waterUV + WaterTranslation * WaterSpeed.y), 0.0f).a;
	float normal4 = NormalHeightMap.SampleLevel(TrilinearWrap, WaterSize.z * (waterUV + WaterTranslation * WaterSpeed.z), 0.0f).a;

	float average = normal1 + normal2 + normal3 + normal4;
	input.WorldPosition.y += (0.25f * average - 0.5f) * WaveHeight * WaveScale;

	output.WorldPosition = mul(input.WorldPosition, WorldMatrix);

	output.ProjectedPosition = mul(output.WorldPosition, ViewMatrix);
	output.ProjectedPosition = mul(output.ProjectedPosition, ProjectionMatrix);

	output.UV = input.UV;

	return output;
}