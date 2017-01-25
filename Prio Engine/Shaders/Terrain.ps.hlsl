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

float GetPercentage(float number, float desiredPercentage)
{
	float onePerc = number / 100.0f;

	return (onePerc * desiredPercentage);
};

// Pixel shader
float4 TerrainPixel(PixelInputType input) : SV_TARGET
{
	float4 textureColour;
	float3 lightDir;
	float lightIntensity;
	float4 colour;

	// Calculate triplanar mapping planes.

	float mXY = abs(input.normal.z);
	float mXZ = abs(input.normal.y);
	float mYZ = abs(input.normal.x);
	float length = mXY + mXZ + mYZ;

	// Normalise the mapping planes.
	mXY /= length;
	mXZ /= length;
	mYZ /= length;

	// The height at which we will say a pixel should have snow on it.
	//const float snowHeight = highestPosition.y - 10.0f;
	const float snowHeight = 85.0f;
	const float grassHeight = 75.0f;
	const float dirtHeight = 70.0f;
	const float sandHeight = 65.0f;
	const float rockHeight = 60.0f;

	const float range = highestPosition.y - lowestPosition.y;

	// If the shader is high up.
	if (input.worldPosition.y > snowHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[1].Sample(SampleType, input.tex);		
		float4 cXY = shaderTexture[1].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[1].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[1].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 snowTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;
		textureColour = snowTex;
	}
	// Blending time.
	else if (input.worldPosition.y > snowHeight - GetPercentage(range, 5.0f))
	{
		float4 cXY = shaderTexture[1].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[1].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[1].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 snowTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ; 
		/* shaderTexture[1].Sample(SampleType, input.tex); */

		cXY = shaderTexture[2].Sample(SampleType, input.worldPosition.xy);
		cXZ = shaderTexture[2].Sample(SampleType, input.worldPosition.xz);
		cYZ = shaderTexture[2].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 grassTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;

		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//float4 grassTex = shaderTexture[2].Sample(SampleType, input.tex);

		float heightDiff = snowHeight - input.worldPosition.y;
		float blendFactor = heightDiff / GetPercentage(range, 5.0f);
		textureColour = lerp(snowTex, grassTex, blendFactor);
	}
	// draw grass texture
	else if (input.worldPosition.y > grassHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[1].Sample(SampleType, input.tex);		
		float4 cXY = shaderTexture[2].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[2].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[2].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 grassTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;
		textureColour = grassTex;
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[2].Sample(SampleType, input.tex);
	}
	// Blend grass with dirt
	else if (input.worldPosition.y > grassHeight - GetPercentage(range, 5.0f))
	{
		float4 cXY = shaderTexture[2].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[2].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[2].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 grassTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ; 

		cXY = shaderTexture[0].Sample(SampleType, input.worldPosition.xy);
		cXZ = shaderTexture[0].Sample(SampleType, input.worldPosition.xz);
		cYZ = shaderTexture[0].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 dirtTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ; 

		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//float4 grassTex = shaderTexture[2].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//float4 dirtTex = shaderTexture[0].Sample(SampleType, input.tex);
		float heightDiff = grassHeight - input.worldPosition.y;
		float blendFactor = heightDiff / GetPercentage(range, 5.0f);
		textureColour = lerp(grassTex, dirtTex, blendFactor);
	}
	// Dirt
	else if (input.worldPosition.y > dirtHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[1].Sample(SampleType, input.tex);		
		float4 cXY = shaderTexture[0].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[0].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[0].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 dirtTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;
		textureColour = dirtTex;

		//textureColour = shaderTexture[0].Sample(SampleType, input.tex);
	}
	// dirt blended with sand
	else if (input.worldPosition.y > dirtHeight - GetPercentage(range, 5.0f))
	{
		float4 cXY = shaderTexture[0].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[0].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[0].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 dirtTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;


		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//float4 dirtTex = shaderTexture[0].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		cXY = shaderTexture[3].Sample(SampleType, input.worldPosition.xy);
		cXZ = shaderTexture[3].Sample(SampleType, input.worldPosition.xz);
		cYZ = shaderTexture[3].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 sandTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;

		//float4 sandTex = shaderTexture[3].Sample(SampleType, input.tex);
		float heightDiff = dirtHeight - input.worldPosition.y;
		float blendFactor = heightDiff / GetPercentage(range, 5.0f);
		textureColour = lerp(dirtTex, sandTex, blendFactor);
	}
	// Sand
	else if (input.worldPosition.y > sandHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[1].Sample(SampleType, input.tex);		
		float4 cXY = shaderTexture[3].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[3].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[3].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 sandTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;
		textureColour = sandTex;

		//textureColour = shaderTexture[3].Sample(SampleType, input.tex);
	}
	// Sand blended with rock.
	else if (input.worldPosition.y > sandHeight - GetPercentage(range, 1.0f))
	{
		float4 cXY = shaderTexture[3].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[3].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[3].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 sandTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;

		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//float4 sandTex = shaderTexture[3].Sample(SampleType, input.tex);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		cXY = shaderTexture[4].Sample(SampleType, input.worldPosition.xy);
		cXZ = shaderTexture[4].Sample(SampleType, input.worldPosition.xz);
		cYZ = shaderTexture[4].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 rockTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;

		//float4 rockTex = shaderTexture[4].Sample(SampleType, input.tex);
		float heightDiff = sandHeight - input.worldPosition.y;
		float blendFactor = heightDiff / GetPercentage(range, 1.0f);
		textureColour = lerp(sandTex, rockTex, blendFactor);
	}
	// Rock.
	else if (input.worldPosition.y > rockHeight)
	{
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		//textureColour = shaderTexture[1].Sample(SampleType, input.tex);		
		float4 cXY = shaderTexture[4].Sample(SampleType, input.worldPosition.xy);
		float4 cXZ = shaderTexture[4].Sample(SampleType, input.worldPosition.xz);
		float4 cYZ = shaderTexture[4].Sample(SampleType, input.worldPosition.yz);
		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		float4 rockTex = cXY * mXY + cXZ * mXZ + cYZ * mYZ;
		textureColour = rockTex;

		//textureColour = shaderTexture[4].Sample(SampleType, input.tex);
	}
	else
	{
		// Default pixel to blue to show any obvious errors (if its lower, blue is okay because it can represent water.
		textureColour.r = 0.0f;
		textureColour.g = 0.0f;
		textureColour.b = 1.0f;
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

	// Return the colour of the current pixel.
	return colour;
}