//////////////////////////
// Sample states
//////////////////////////

SamplerState TrilinearWrap : register(s0);
SamplerState BilinearMirror : register(s1);

//////////////////////////
// Textures
//////////////////////////

Texture2D NormalHeightMap : register(t0);
Texture2D RefractionMap : register(t1);
Texture2D ReflectionMap : register(t2);

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

cbuffer CameraBuffer : register(b2)
{
	matrix	CameraMatrix;
	float3	CameraPosition;
	float	cameraPadding;
}

cbuffer ViewportBuffer : register(b3)
{
	float2 ViewportSize;
}

cbuffer LightBuffer : register(b4)
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
	float2 UV : TEXCOORD0;
	float4 WorldPosition : POSITION;
};


//////////////////////////
// Pixel shader
//////////////////////////

float4 WaterSurfacePS(PixelInputType input) : SV_TARGET
{
	float2 waterUV = input.UV;
	float3 normal1 = NormalHeightMap.Sample(TrilinearWrap, WaterSize.w * (waterUV + WaterTranslation * WaterSpeed.w)).rgb * 2.0f - 1.0f;
	float3 normal2 = NormalHeightMap.Sample(TrilinearWrap, WaterSize.x * (waterUV + WaterTranslation * WaterSpeed.x)).rgb * 2.0f - 1.0f;
	float3 normal3 = NormalHeightMap.Sample(TrilinearWrap, WaterSize.y * (waterUV + WaterTranslation * WaterSpeed.y)).rgb * 2.0f - 1.0f;
	float3 normal4 = NormalHeightMap.Sample(TrilinearWrap, WaterSize.z * (waterUV + WaterTranslation * WaterSpeed.z)).rgb * 2.0f - 1.0f;

	normal1.y *= WaterSize.w;
	normal2.y *= WaterSize.x;
	normal3.y *= WaterSize.y;
	normal4.y *= WaterSize.z;

	normal1 = normalize(normal1);
	normal2 = normalize(normal2);
	normal3 = normalize(normal3);
	normal4 = normalize(normal4);

	float3 waterNormal = (normal1 + normal2 + normal3 + normal4) / 4.0f;

	float temp = waterNormal.z;
	waterNormal.z = waterNormal.y;
	waterNormal.y = temp;

	waterNormal.y /= (WaveScale + 0.001f);

	//normalise final normal value
	waterNormal = normalize(waterNormal);

	float2 waterNormal2D = waterNormal.xz;
	float2 offsetDir = waterNormal2D;

	float2 screenUV = input.ProjectedPosition.xy / ViewportSize;
	float4 refractionDepth = RefractionMap.Sample(BilinearMirror, screenUV).a;
	float4 reflectionHeight = ReflectionMap.Sample(BilinearMirror, screenUV).a;

	float2 refractionUV = screenUV + RefractionDistortion * refractionDepth * offsetDir / input.ProjectedPosition.w;
	float2 reflectionUV = screenUV + ReflectionDistortion * reflectionHeight * offsetDir / input.ProjectedPosition.w;
	float4 refractColour = RefractionMap.Sample(BilinearMirror, refractionUV) * RefractionStrength;
	float4 reflectColour = ReflectionMap.Sample(BilinearMirror, reflectionUV) * ReflectionStrength;

	reflectColour = lerp(refractColour, reflectColour, saturate(refractionDepth * MaxDistortionDistance / (0.5f * WaveHeight * WaveScale)));

	float3 normalToCamera = normalize(CameraPosition - input.WorldPosition.xyz);

	// Light 1
	float3 vectorToLight = -LightDirection;
	float3 normalToLight = normalize(vectorToLight);
	float3 halfwayVector = normalize(normalToLight + normalToCamera);
	float3 specularLight = 0.0f;

	reflectColour.rgb += 2.0f * specularLight;


	float n1 = 1.0;
	float n2 = 1.5f;
	float  f0 = pow(((n1 - n2) / (n1 + n2)),2);
	float fresnel = 0.25;

	return lerp(refractColour, reflectColour, fresnel);
}