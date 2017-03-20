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
// Make the terrain shader here match the normal terrain shader
// Here for now, we're just going to make it all the same texture so we can demonstrate refraction working or not. Don't really give a fuck 
// if it reflects the real world for now. Working first, optimise later.
//////////////////////////

float4 TerrainRefractionPS(PixelInputType input) : SV_TARGET
{
	float2 screenUV = input.ProjectedPosition.xy / ViewportSize;
	float waterHeight = WaterHeightMap.Sample(BilinearMirror, screenUV);
	float objectDepth = waterHeight - input.WorldPosition.y;

	float3 maxLightDistance = float3(15.0f,75.0f,300.0f);
	float waterBrightness = 0.5f;
	float maxDistortionDistance = 40.0f;

	clip(objectDepth);

	float3 sceneColour = ShaderTexture.Sample(TrilinearWrap, input.UV).rgb;

	float3 depthDarken = saturate(objectDepth / maxLightDistance);

	float3 refractionColour = lerp(sceneColour, normalize(maxLightDistance) * waterBrightness, depthDarken);

	return float4(refractionColour, saturate(objectDepth / maxDistortionDistance));
}