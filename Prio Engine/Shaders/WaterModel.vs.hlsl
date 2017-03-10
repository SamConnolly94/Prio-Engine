/////////////////////
// Constant Buffers
////////////////////

cbuffer MatrixBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjMatrix;
};

///////////////////////
// Input types
//////////////////////

// The structure of the data passed into the 
struct VertexInputType
{
	float3 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
};

struct PixelInputType
{
	float4 ProjPosition : SV_POSITION;
	float3 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
};

//////////////////////
// Vertex shader
/////////////////////

PixelInputType WaterModelVS(VertexInputType input)
{
	// Define the pixel we're going to feed into the pixel shader.
	PixelInputType output;

	// Transform our world position into a float 4 with a 4th value of 1 so we can multiply it by the view and proj matrix to get the screen position.
	float4 worldPos = float4(input.WorldPosition, 1.0f);
	
	// Perform the conversion from world space to screen space.
	output.WorldPosition = mul(worldPos, WorldMatrix);
	output.ProjPosition = mul(output.WorldPosition, ViewMatrix);
	output.ProjPosition = mul(output.ProjPosition, ProjMatrix);

	// Cast the normal to world space.
	output.Normal = mul(input.Normal, (float3x3)WorldMatrix);

	// Copy the same tex coords from the vertex passed in and pass them forward to the pixel shader.
	output.UV = input.UV;

	return output;
}