///////////////////////////
// Terrain pixel shader.
// Author: Sam Connolly
// Last update: 24/01/2017
///////////////////////////

// 1) Dirt
// 2) Snow
// 3) Yellow Grass
// 4) Sand
// 5) Rock
Texture2D shaderTexture[5];
SamplerState SampleType;

///////////////////////////
// Buffers

cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
};

cbuffer TerrainInfoBuffer : register(b1)
{
	float3 highestPosition;
	float3 lowestPosition;
	float2 padding2;
};

///////////////////////////
// Typedefs

struct PixelInputType
{
	float4 screenPosition : SV_POSITION;
	float4 worldPosition : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

// Pixel shader
float4 TerrainPixel(PixelInputType input) : SV_TARGET
{
	float4 textureColour;
	float3 lightDir;
	float lightIntensity;
	float4 colour;
	// The height at which we will say a pixel should have snow on it.
	//const float snowHeight = highestPosition.y - 10.0f;
	//const float snowHeight = 85.0f;
	//const float grassHeight = 75.0f;
	//const float dirtHeight = 70.0f;
	//const float sandHeight = 65.0f;
	//const float rockHeight = 60.0f;
	//const float range = highestPosition.y - lowestPosition.y;
	const float snowHeight = highestPosition.y - 2.0f;
	const float grassHeight = highestPosition.y - 10.0f;
	const float dirtHeight = highestPosition.y - 15.0f;
	const float sandHeight = highestPosition.y - 20.0f;
	const float rockHeight = lowestPosition.y;

	// If the shader is high up.
	if (input.worldPosition.y > snowHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		textureColour = shaderTexture[1].Sample(SampleType, input.tex);
	}
	// Blending time.
	else if (input.worldPosition.y > snowHeight - 2.0f)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 snowTex = shaderTexture[1].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 grassTex = shaderTexture[2].Sample(SampleType, input.tex);
		float heightDiff = snowHeight - input.worldPosition.y;
		float blendFactor = heightDiff / 2.0f;
		textureColour = lerp(snowTex, grassTex, blendFactor);
	}
	// draw grass texture
	else if (input.worldPosition.y > grassHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		textureColour = shaderTexture[2].Sample(SampleType, input.tex);
	}
	// Blend grass with dirt
	else if (input.worldPosition.y > grassHeight - 5.0f)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 grassTex = shaderTexture[2].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 dirtTex = shaderTexture[0].Sample(SampleType, input.tex);
		float heightDiff = grassHeight - input.worldPosition.y;
		float blendFactor = heightDiff / 5.0f;
		textureColour = lerp(grassTex, dirtTex, blendFactor);
	}
	// Dirt
	else if (input.worldPosition.y > dirtHeight)
	{
		textureColour = shaderTexture[0].Sample(SampleType, input.tex);
	}
	// dirt blended with sand
	else if (input.worldPosition.y > dirtHeight - 5.0f)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 dirtTex = shaderTexture[0].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 sandTex = shaderTexture[3].Sample(SampleType, input.tex);
		float heightDiff = dirtHeight - input.worldPosition.y;
		float blendFactor = heightDiff / 5.0f;
		textureColour = lerp(dirtTex, sandTex, blendFactor);
	}
	// Sand
	else if (input.worldPosition.y > sandHeight)
	{
		textureColour = shaderTexture[3].Sample(SampleType, input.tex);
	}
	// Sand blended with rock.
	else if (input.worldPosition.y > sandHeight - 5.0f)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 sandTex = shaderTexture[3].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 rockTex = shaderTexture[4].Sample(SampleType, input.tex);
		float heightDiff = sandHeight - input.worldPosition.y;
		float blendFactor = heightDiff / (5.0f);
		textureColour = lerp(sandTex, rockTex, blendFactor);
	}
	// Rock.
	else if (input.worldPosition.y > rockHeight)
	{
		textureColour = shaderTexture[4].Sample(SampleType, input.tex);
	}
	else
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[2].Sample(SampleType, input.tex);
		textureColour.rgb = (1.0f, 1.0f, 1.0f);
		textureColour.a = 1.0f;
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

	//// If the shader is high up.
	//if (input.worldPosition.y > snowHeight)
	//{
	//	// Set the colour to be white.
	//	colour.rgb = 1.0f, 1.0f, 1.0f;
	//	colour.w = 1.0f;
	//};

	// Return the colour of the current pixel.
	return colour;
}