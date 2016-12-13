// Vertex shader for the font which will be used to draw things in game.

// Globals
cbuffer matrices
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

// Type definitions.
struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType FontVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change pos to have a 4th element so can be used in matrix multiplication.
	input.position.w = 1.0f;

	// Calculate pos of vertex against world, view and proj matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}