//////////////////////////////
// Textures
/////////////////////////////

Texture2D WaterHeightMap : register(t0);
Texture2D TerrainTexture : register(t3);

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
	float4 ProjPosition : SV_POSITION;
	float3 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
};


float4 PixelLightingPS(PixelInputType input) : SV_TARGET
{
	// By this point normals have been transformed by the world matrix, which might contain scaling, and interpolated by the
	// rasterizer stage, which also might introduce scaling. In other words they are not likely length 1 any more, so renormalise
	float3 worldNormal = normalize(input.Normal);

	///////////////////////
	// Calculate lighting

	// Calculate vector to camera from world position of current pixel
	float3 normalToCamera = normalize(CameraPos - input.WorldPosition);

	// Light 1
	float3 vectorToLight = LightPosition - input.WorldPosition; // Vector to light from current pixel
	float3 normalToLight = normalize(vectorToLight);
	float3 halfwayVector = normalize(normalToLight + normalToCamera);
	float3 diffuseLight1 = DiffuseColour  * max(dot(worldNormal, normalToLight), 0) / length(vectorToLight);
	float3 specularLight1 = diffuseLight1 * pow(max(dot(worldNormal, halfwayVector), 0), SpecularPower);

	// Sum the effect of the two lights and add an ambient light level (to the diffuse)
	float3 diffuseLight = AmbientColour + diffuseLight1;
	float3 specularLight = specularLight1;


	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture
	float4 diffuseMaterial = TerrainTexture.Sample(TrilinearWrap, input.UV);

	// Get specular material colour from texture alpha
	float3 specularMaterial = diffuseMaterial.a;

	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = diffuseMaterial * diffuseLight + specularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 ReflectionPS(PixelInputType input) : SV_TARGET
{
	float2 screenUV = input.ProjPosition.xy / ViewportSize;
	float waterHeight = WaterHeightMap.Sample(BilinearMirror, screenUV);
	float objectHeight = input.WorldPosition.y - (2 * WaterPlaneY - waterHeight);

	clip(objectHeight);

	return float4 (PixelLightingPS(input).rgb, saturate(objectHeight / MaxDistortionDistance));
}