Texture2D shaderTexture;
SamplerState SampleType;

///////////////////////////
// Terrain pixel shader.
// Author: Sam Connolly
// Last update: 23/01/2017
///////////////////////////

///////////////////////////
// Buffers

cbuffer LightBuffer
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
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
	const float snowHeight = 80.0f;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColour = shaderTexture.Sample(SampleType, input.tex);

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

	// If the shader is high up.
	if (input.worldPosition.y > snowHeight)
	{
		// Set the colour to be white.
		colour.rgb = 1.0f, 1.0f, 1.0f;
		colour.w = 1.0f;
	};

	// Return the colour of the current pixel.
	return colour;
}