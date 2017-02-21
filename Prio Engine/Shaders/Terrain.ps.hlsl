///////////////////////////
// Terrain pixel shader.
// Author: Sam Connolly
// Last update: 25/01/2017
///////////////////////////

//////////////////////////
// 1) Dirt
// 2) Snow
// 3) Sand
// 4) Rock
/////////////////////////
Texture2D shaderTexture[2];
Texture2D grassTextures[2];
Texture2D patchMap;
Texture2D rockTextures[2];
SamplerState SampleType;

///////////////////////////
// Buffers
///////////////////////////

/* The buffer which contains details about our lighting information. */
cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
};

/* The buffer which contains information about our terrain. */
cbuffer TerrainInfoBuffer : register(b1)
{
	float highestPosition;
	float lowestPosition;
	float2 terrBuffPadding;
};

/* Information about the model of the terrain itself. */
// Important note - separate from terrainInfoBuffer due to the way in which padding works.
cbuffer PositioningBuffer : register(b2)
{
	float yOffset;
	float3 posPadding;
}

//////////////////////
// Typedefs
/////////////////////

/* The pixel input type is the information that will be passed in from the vertex shader.
*  This will need to match the structure in the vertex shader. */
struct PixelInputType
{
	float4 screenPosition : SV_POSITION;
	float4 worldPosition : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

///////////////////////
// Helper Functions
///////////////////////

/* Get the percentage of a number.
*  Param number - The number which you want to find the percentage of.
*  Param desiredPercentage - The percentage of the number which you wish to find. */
float GetPercentage(float number, float desiredPercentage)
{
	float onePerc = number / 100.0f;

	return (onePerc * desiredPercentage);
};

float4 GetTriplanarTextureColour(int texIndex, float3 blending, float4 worldPosition, float scale)
{

	scale = 0.1f;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	float4 xAxis = shaderTexture[texIndex].Sample(SampleType, worldPosition.yz * scale);
	float4 yAxis = shaderTexture[texIndex].Sample(SampleType, worldPosition.xz * scale);
	float4 zAxis = shaderTexture[texIndex].Sample(SampleType, worldPosition.xy * scale);

	//float4 xAxisTimes4 = shaderTexture[texIndex].Sample(SampleType, worldPosition.yz * (scale * 4));
	//float4 yAxisTimes4 = shaderTexture[texIndex].Sample(SampleType, worldPosition.xz * (scale * 4));
	//float4 zAxisTimes4 = shaderTexture[texIndex].Sample(SampleType, worldPosition.xy * (scale * 4));

	//float4 xAxisTimes16 = shaderTexture[texIndex].Sample(SampleType, worldPosition.yz * (scale * 16));
	//float4 yAxisTimes16 = shaderTexture[texIndex].Sample(SampleType, worldPosition.xz * (scale * 16));
	//float4 zAxisTimes16 = shaderTexture[texIndex].Sample(SampleType, worldPosition.xy * (scale * 16));

	float4 result;
	result = xAxis * blending.x + yAxis * blending.y + zAxis * blending.z;
	//result += xAxisTimes4 * blending.x + yAxisTimes4 * blending.y + zAxisTimes4 * blending.z;
	//result += xAxisTimes16 * blending.x + yAxisTimes16 * blending.y + zAxisTimes16 * blending.z;
	//result /= 3;

	return result;
};

float4 GetTriplanarGrassTextureColour(int texIndex, float3 blending, float4 worldPosition, float scale)
{
	scale = 0.1f;
	float4 xAxis = grassTextures[texIndex].Sample(SampleType, worldPosition.yz * scale);
	float4 yAxis = grassTextures[texIndex].Sample(SampleType, worldPosition.xz * scale);
	float4 zAxis = grassTextures[texIndex].Sample(SampleType, worldPosition.xy * scale);

	//float4 xAxisTimes4 = grassTextures[texIndex].Sample(SampleType, worldPosition.yz * (scale * 4));
	//float4 yAxisTimes4 = grassTextures[texIndex].Sample(SampleType, worldPosition.xz * (scale * 4));
	//float4 zAxisTimes4 = grassTextures[texIndex].Sample(SampleType, worldPosition.xy * (scale * 4));

	//float4 xAxisTimes16 = grassTextures[texIndex].Sample(SampleType, worldPosition.yz * (scale * 16));
	//float4 yAxisTimes16 = grassTextures[texIndex].Sample(SampleType, worldPosition.xz * (scale * 16));
	//float4 zAxisTimes16 = grassTextures[texIndex].Sample(SampleType, worldPosition.xy * (scale * 16));

	float4 result;
	result = xAxis * blending.x + yAxis * blending.y + zAxis * blending.z;
	//result += xAxisTimes4 * blending.x + yAxisTimes4 * blending.y + zAxisTimes4 * blending.z;
	//result += xAxisTimes16 * blending.x + yAxisTimes16 * blending.y + zAxisTimes16 * blending.z;
	//result /= 3;

	return result;

}


float4 GetPatchGrassColour(PixelInputType input, float3 blending)
{
	float4 grass1 = GetTriplanarGrassTextureColour(0, blending, input.worldPosition, 1.0f);
	float4 grass2 = GetTriplanarGrassTextureColour(1, blending, input.worldPosition, 1.0f);

	float4 patchColour = patchMap.Sample(SampleType, input.tex / 32.0f);

	float4 textureColour = lerp(grass2, grass1, patchColour.r);

	return textureColour;
}

float4 GetTriplanarRockTextureColour(int texIndex, float3 blending, float4 worldPosition, float scale)
{
	scale = 0.1f;
	float4 xAxis = rockTextures[texIndex].Sample(SampleType, worldPosition.yz * scale);
	float4 yAxis = rockTextures[texIndex].Sample(SampleType, worldPosition.xz * scale);
	float4 zAxis = rockTextures[texIndex].Sample(SampleType, worldPosition.xy * scale);

	//float4 xAxisTimes4 = rockTextures[texIndex].Sample(SampleType, worldPosition.yz * (scale * 4));
	//float4 yAxisTimes4 = rockTextures[texIndex].Sample(SampleType, worldPosition.xz * (scale * 4));
	//float4 zAxisTimes4 = rockTextures[texIndex].Sample(SampleType, worldPosition.xy * (scale * 4));

	//float4 xAxisTimes16 = rockTextures[texIndex].Sample(SampleType, worldPosition.yz * (scale * 16));
	//float4 yAxisTimes16 = rockTextures[texIndex].Sample(SampleType, worldPosition.xz * (scale * 16));
	//float4 zAxisTimes16 = rockTextures[texIndex].Sample(SampleType, worldPosition.xy * (scale * 16));

	float4 result;
	result = xAxis * blending.x + yAxis * blending.y + zAxis * blending.z;
	//result += xAxisTimes4 * blending.x + yAxisTimes4 * blending.y + zAxisTimes4 * blending.z;
	//result += xAxisTimes16 * blending.x + yAxisTimes16 * blending.y + zAxisTimes16 * blending.z;
	//result /= 3;

	return result;

}

float4 GetCombinedRockLerp(PixelInputType input, float3 blending)
{
	float4 rock1 = GetTriplanarRockTextureColour(0, blending, input.worldPosition, 1.0f);
	float4 rock2 = GetTriplanarRockTextureColour(1, blending, input.worldPosition, 1.0f);

	float4 patchColour = patchMap.Sample(SampleType, input.tex / 32.0f);

	float4 textureColour = lerp(rock2, rock1, patchColour.r);

	return textureColour;
}


///////////////////////////
// Pixel Shading Functions
///////////////////////////

/* This is the main body of our pixel shader and our entry point.
*  The purpose of this is to shade the pictures using a different texture depending on the height of the current vertex.
*  So far we have implemented Snow, Grass, Dirt, Sand, Rocks and at the bottom below the rocks there are some pools of water. */
float4 TerrainPixel(PixelInputType input) : SV_TARGET
{
	float4 textureColour;
	float3 lightDir;
	float lightIntensity;
	float4 colour;

	// Get normals on different planes
	float3 blending = abs(input.normal);
	// Make sure the blending weight is of length 1.
	blending = normalize(max(blending, 0.00001));
	float blendLen = (blending.x + blending.y + blending.z);
	blending /= (blendLen, blendLen, blendLen);

	// Find the world position by moving the vertex by whatever our current Y position of the terrain is.
	// This only needs to be done as we support movement of terrain in Prio Engine.
	float worldPos = input.worldPosition.y + yOffset;

	const float changeInHeight = highestPosition - lowestPosition;

	// Define the position 
	const float snowHeight = lowestPosition + GetPercentage(changeInHeight, 60);	// 60% and upwards will be snow.
	const float grassHeight = lowestPosition + GetPercentage(changeInHeight, 30);	// 30% and upwards will be grass.
	const float dirtHeight = lowestPosition + GetPercentage(changeInHeight, 15);	// 15% and upwards will be dirt.
	const float sandHeight = lowestPosition + GetPercentage(changeInHeight, 10);	// 10% and upwards will be sand.
	const float rockHeight = lowestPosition + GetPercentage(changeInHeight, 4);		// 4%  and upwards will be rocks.

	float4 patchColour = patchMap.Sample(SampleType, input.tex);

	/////////// SNOW //////////////////
	if (worldPos > snowHeight)
	{
		//textureColour = GetTriplanarTextureColour(1, blending, input.worldPosition, 1.0f);
		textureColour = GetCombinedRockLerp(input, blending);
	}
	///////////// SNOW BLENDED WITH GRASS /////////////
	else if (worldPos > snowHeight - 5.0f)
	{
		//float4 snowTex = GetTriplanarTextureColour(1, blending, input.worldPosition, 1.0f);
		float4 rockTex = GetCombinedRockLerp(input, blending);
		float4 grassTex = GetPatchGrassColour(input, blending);
		float heightDiff = snowHeight - worldPos;
		float blendFactor = heightDiff / 5.0f;
		textureColour = lerp(rockTex, grassTex, blendFactor);
	}
	////////////// GRASS ///////////////////
	else if (worldPos > grassHeight)
	{
		textureColour = GetPatchGrassColour(input, blending);
	}
	////////////////// GRASS BLENDED WITH DIRT ////////////////
	else if (worldPos > grassHeight - 2.0f)
	{
		float4 grassTex = GetPatchGrassColour(input, blending);
		float4 dirtTex = GetTriplanarTextureColour(0, blending, input.worldPosition, 1.0f);
		float heightDiff = grassHeight - worldPos;
		float blendFactor = heightDiff / 2.0f;
		textureColour = lerp(grassTex, dirtTex, blendFactor);
	}
	/////////////////////// DIRT //////////////////////
	else if (worldPos > dirtHeight)
	{
		textureColour = GetTriplanarTextureColour(0, blending, input.worldPosition, 1.0f);
	}
	////////////////////// DIRT BLENDED WITH SAND ///////////////
	else if (worldPos > dirtHeight - 2.0f)
	{
		float4 dirtTex = GetTriplanarTextureColour(0, blending, input.worldPosition, 1.0f);
		float4 sandTex = GetTriplanarTextureColour(1, blending, input.worldPosition, 1.0f);
		float heightDiff = dirtHeight - worldPos;
		float blendFactor = heightDiff / 2.0f;
		textureColour = lerp(dirtTex, sandTex, blendFactor);
	}
	//////////////////// SAND //////////////////
	else if (worldPos > sandHeight)
	{
		textureColour = GetTriplanarTextureColour(1, blending, input.worldPosition, 1.0f);
	}
	// Below all height, assume sand.
	else
	{
		textureColour = GetTriplanarTextureColour(1, blending, input.worldPosition, 1.0f);
	}

	// Set the colour to the ambient colour.
	colour = ambientColour;

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		colour += (diffuseColour * lightIntensity);
	}

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	colour = saturate(colour);

	// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
	colour = colour * textureColour;

	// Return the colour of the current pixel.
	return colour;
}