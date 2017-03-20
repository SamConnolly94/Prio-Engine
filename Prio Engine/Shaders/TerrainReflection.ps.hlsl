//////////////////////////
// Sample states
//////////////////////////

SamplerState TrilinearWrap : register(s0);
SamplerState BilinearMirror : register(s1);

//////////////////////////
// Textures
//////////////////////////

Texture2D WaterHeightMap : register(t0);
Texture2D ShaderTexture : register(t1);

//////////////////////////
// Constant buffers
//////////////////////////

cbuffer ViewportBuffer : register(b0)
{
	float2 ViewportSize;
}

cbuffer LightBuffer : register(b1)
{
	float4	AmbientColour;
	float4	DiffuseColour;
	float3	LightDirection;
	float	lightBufferPadding;
}

//////////////////////////
// Structures
//////////////////////////

struct PixelInputType
{
	float4 ProjectedPosition : SV_POSITION;
	float4 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
};


//////////////////////////
// Pixel shader
// TODO: 
//////////////////////////

float4 TerrainReflectionPS(PixelInputType input) : SV_TARGET
{
	float2 screenUV = input.ProjectedPosition.xy / ViewportSize;
	float waterHeight = WaterHeightMap.Sample(BilinearMirror, screenUV);
	float objectHeight = input.WorldPosition.y - (30.0f - waterHeight);
	clip(objectHeight);

	float4 colour = ShaderTexture.Sample(TrilinearWrap, input.UV);
	return float4(colour.rgb, saturate(objectHeight / 40.0f)); //Saturate the colour based on the height of the object 
}