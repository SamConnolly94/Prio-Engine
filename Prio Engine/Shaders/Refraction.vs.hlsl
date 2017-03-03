cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
};

cbuffer ClipPlaneBuffer
{
	float4 clipPlane;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
};

PixelInputType RefractionVS(VertexInputType input)
{
	PixelInputType output;

	// Give the pos a 4th value so we can multiply it with matrices.
	input.position.w = 1.0f;

	// Calculate the position of the pixel as it will appear on the screen.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);

	// Store tex coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal in the world matrix.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

	output.clip = dot(mul(input.position, worldMatrix), clipPlane);

	return output;
}