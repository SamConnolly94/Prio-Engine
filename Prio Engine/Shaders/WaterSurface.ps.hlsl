/////////////////////////////////////
// Author: Sam Connolly
// Date: 09/03/2017
// Purpose: To draw the height map of the water onto a texture, this will describe how the water should currently look.
////////////////////////////////////

//////////////////////////////
// Textures
/////////////////////////////

Texture2D WaterHeightMap : register(t0);
Texture2D RefractionMap : register(t1);
Texture2D ReflectionMap : register(t2);

//////////////////////
// Sampler states.
//////////////////////

SamplerState TrilinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState BilinearMirror
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	MaxLOD = 0;
	AddressU = Mirror;
	AddressV = Mirror;
};

//////////////////////////
// Constant buffers
/////////////////////////

cbuffer WaterBuffer : register(b0)
{
	float2 WaterMovement;
	float WaveScale;
	float RefractionStrength;
	float ReflectionStrength;
	float MaxDistortionDistance;
	float WaterPlaneY;
	float waterBufferPadding;
};

cbuffer CameraBuffer : register(b1)
{
	matrix CameraMatrix;
	float3 CameraPos;
	float cameraBufferPadding;
};

cbuffer LightBuffer : register(b2)
{
	float3 LightPosition;
	float4 AmbientColour;
	float4 DiffuseColour;
	float3 LightDirection;
	float SpecularPower;
	float4 SpecularColor;
	float lightBufferPadding;
};

cbuffer ViewportBuffer : register(b3)
{
	// Viewport size in pixels
	float2 ViewportSize;
	float2 viewportPadding;
}

//////////////////////////
// Pixel input structures
/////////////////////////

struct PixelInputType
{
	float4 ProjPos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 tex : TEXCOORD0;
};

//////////////////////
// Globals
///////////////////////

// Water normal map/height map is sampled four times at different sizes, then overlaid to give a complex wave pattern
static const float WaterSize1 = 0.5f;
static const float WaterSize2 = 1.0f;
static const float WaterSize3 = 2.0f;
static const float WaterSize4 = 4.0f;

static const float WaterSpeed1 = 0.5f;
static const float WaterSpeed2 = 1.0f;
static const float WaterSpeed3 = 1.7f;
static const float WaterSpeed4 = 2.6f;

static const float  RefractionDistortion = 20.0f;

static const float HeightMapHeightOverWidth = 1 / 32.0f;
static const float WaterWidth = 400.0f;
static const float MaxWaveHeight = WaterWidth * HeightMapHeightOverWidth;
static const float  WaterRefractiveIndex = 1.5f;  // Refractive index of clean water is 1.33. Impurities increase this value and values up to about 7.0 are sensible

////////////////////////
// Shader 
///////////////////////

float4 WaterPS(PixelInputType input) : SV_TARGET
{
	float3 normal1 = WaterHeightMap.Sample(TrilinearWrap, WaterSize1 * (input.tex + WaterMovement * WaterSpeed1)).rgb * 2.0f - 1.0f;
	float3 normal2 = WaterHeightMap.Sample(TrilinearWrap, WaterSize2 * (input.tex + WaterMovement * WaterSpeed2)).rgb * 2.0f - 1.0f;
	float3 normal3 = WaterHeightMap.Sample(TrilinearWrap, WaterSize3 * (input.tex + WaterMovement * WaterSpeed3)).rgb * 2.0f - 1.0f;
	float3 normal4 = WaterHeightMap.Sample(TrilinearWrap, WaterSize4 * (input.tex + WaterMovement * WaterSpeed4)).rgb * 2.0f - 1.0f;

	normal1.y *= WaterSize1;
	normal2.y *= WaterSize2;
	normal3.y *= WaterSize3;
	normal4.y *= WaterSize4;

	float3 waterNormal = float3(0, 1, 0); // Not this, refer to laurents water lab

	waterNormal.y /= (WaveScale + 0.001f);
	waterNormal = normalize(waterNormal); 

	float2 waterNormal2D = waterNormal.xz;
	float2 offsetDir = float2(dot(waterNormal2D, normalize(CameraMatrix[0].xz)), dot(waterNormal2D, normalize(CameraMatrix[2].xz)));

	float2 screenUV = input.ProjPos.xy / ViewportSize;
	float4 refractionDepth = RefractionMap.Sample(BilinearMirror, screenUV).a;
	float4 reflectionHeight = ReflectionMap.Sample(BilinearMirror, screenUV).a;

	float2 refractionUV = screenUV + RefractionDistortion * refractionDepth  * offsetDir / input.ProjPos.w;
	float2 reflectionUV = screenUV; // Needs more code on this line, refer to laurents water lab
	float4 refractColour = RefractionMap.Sample(BilinearMirror, refractionUV) * RefractionStrength;
	float4 reflectColour = ReflectionMap.Sample(BilinearMirror, reflectionUV) * ReflectionStrength;
	reflectColour = lerp(refractColour, reflectColour, saturate(refractionDepth * MaxDistortionDistance / (0.5f * MaxWaveHeight * WaveScale)));
	float3 normalToCamera = normalize(CameraPos - input.WorldPos);

	float3 vectorToLight = LightPosition - input.WorldPos;
	float3 normalToLight = normalize(vectorToLight);
	float3 halfwayVector = normalize(normalToLight + normalToCamera);
	float3 specularLight1 = LightPosition * pow(max(dot(waterNormal, halfwayVector), 0), SpecularPower) / length(vectorToLight);

	reflectColour.rgb += SpecularPower * (specularLight1);

	float n1 = 1.0;
	float n2 = WaterRefractiveIndex;
	float fresnel = 0.25f;

	return lerp(refractColour, reflectColour, fresnel);
}