Texture2D shaderTexture;
SamplerState SamplerType;

cbuffer PixelBuffer
{
	float4 pixelColour;
};

// Type definitions
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

// Pixel shader.
float4 FontPixelShader(PixelInputType input) : SV_TARGET
{
	float4 colour;

	// Sample the texture pixel at this location.
	colour = shaderTexture.Sample(SamplerType, input.tex);

	// If the colour is solid black on this texture then treat it as transparent.
	if (colour.r == 0.0f && colour.g == 0.0f && colour.b == 0.0f)
	{
		colour.a = 0.0f;
	}
	else
	{
		colour.a = 1.0f;
		colour = colour * pixelColour;
	}

	return colour;
}