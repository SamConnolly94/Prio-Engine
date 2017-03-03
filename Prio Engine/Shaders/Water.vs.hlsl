cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
};

cbuffer ReflectionBuffer
{
	matrix reflectionMatrix;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 reflectionPosition : TEXCOORD1;
	float4 refractionPosition : TEXCOORD2;
};

// Vertex shader
PixelInputType WaterVS(VertexInputType input)
{
	PixelInputType output;
	matrix reflectProjWorld;
	matrix viewProjWorld;

	input.position.w = 1.0f;

	// Calculate the pos of vertex.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);

	output.tex = input.tex;

	reflectProjWorld = mul(reflectionMatrix, projMatrix);
	reflectProjWorld = mul(worldMatrix, reflectProjWorld);

	output.reflectionPosition = mul(input.position, reflectProjWorld);
	
	viewProjWorld = mul(viewMatrix, projMatrix);
	viewProjWorld = mul(worldMatrix, viewProjWorld);

	output.refractionPosition = mul(input.position, viewProjWorld);

	return output;
}