//////////////////////////////
// Textures
/////////////////////////////

Texture2D WaterHeightMap : register(t0);
Texture2D RefractionMap : register(t1);
Texture2D ReflectionMap : register(t2);
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

///////////////////////////////
// Constants
/////////////////////////////
static const float3 WaterExtinction = float3(15, 75, 300);
static const float  WaterDiffuseLevel = 0.5f;


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

// Per-pixel lighting for refracted objects - only renders below water
float4 RefractionPS(PixelInputType input) : SV_Target
{
	// Sample the height map of the water to find if this pixel is underwater
	float2 screenUV = input.ProjPosition.xy / ViewportSize;
	float waterHeight = WaterHeightMap.Sample(BilinearMirror, screenUV);
	float objectDepth = waterHeight - input.WorldPosition.y;
	clip(objectDepth); // Remove pixels with negative depth - i.e. above the water

					   // Get the basic colour for this pixel by calling the standard pixel-lighting shader
	float3 sceneColour = PixelLightingPS(input).rgb;

	// Darken the colour based on the depth underwater
	// TODO - STAGE 1: Darken deep water
	//                 Use the < and > keys to adjust the water height, we would like objects going deep underwater to darken and
	//                 become blue as happens will real water. The second line below (refractionColour = ...) does the colour tint, 
	//                 but first you need to calculate a factor of how much to darken. There is a constant "WaterExtinction" at the
	//                 top of the file. It sets how many metres red, blue and green light can travel in water. We also have the
	//                 depth underwater of the current pixel in the variable "objectDepth":
	//                 - Formula needed in line below is simple, object depth divided by water extinction level
	//                 - However, ensure the result does not exceed 1. There is a HLSL function to do this better than an if, but
	//                   use an "if" if you don't remember it.
	//                 When finished ensure objects underwater darken to blue
	float3 depthDarken = 0; // Not 0, read comment above
	float3 refractionColour = lerp(sceneColour, normalize(WaterExtinction) * WaterDiffuseLevel, depthDarken);

	// Store the darkened colour in rgb and a value representing how deep the pixel is in alpha (used for refraction distortion)
	// This is written to a 8-bit RGBA texture, so this alpha depth value has limited accuracy
	return float4(refractionColour, saturate(objectDepth / MaxDistortionDistance)); // Alpha ranges 0->1 for depths of 0 to MaxDistortionDistance
}