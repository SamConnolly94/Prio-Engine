///////////////////////////
// Buffers
///////////////////////////

cbuffer MatrixBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix ViewProjMatrix;
};

cbuffer FoliageBuffer : register(b1)
{
	float3 WindDirection;
	float FrameTime;
	float3 FoliageTranslation;
	float WindStrength;
};

///////////////////////////
// Input Structures
///////////////////////////

struct VertexInputType
{
	float4 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
	uint IsTopVertex : TEXCOORD1;
	uint Type : TEXCOORD2;
};

struct PixelInputType
{
	float4 ScreenPosition : SV_POSITION;
	float4 WorldPosition : POSITION;
	float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
	uint Type : TEXCOORD1;
};

///////////////////////////
// Vertex shader
///////////////////////////
#define GrassType 0

PixelInputType FoliageVS(VertexInputType input)
{
	PixelInputType output;

	// Give a 4th element to our matrix so it's the correct size;
	input.WorldPosition.w = 1.0f;
	
	if (input.IsTopVertex == 1 && input.Type == GrassType)
	{
		input.WorldPosition.xyz += FoliageTranslation;
	}

	output.WorldPosition = input.WorldPosition;

	// Calculate the position of the vertex against the world, view and projection matrices.
	output.ScreenPosition = mul(input.WorldPosition, WorldMatrix);
	output.ScreenPosition = mul(output.ScreenPosition, ViewProjMatrix);

	// Store the texture coordinates for the pixel shader.
	output.UV = input.UV;

	// Calculate the normal vector against the world matrix only.
	output.Normal = mul(input.Normal, (float3x3)WorldMatrix);

	// Normalise the vector.
	output.Normal = normalize(output.Normal);

	output.Type = input.Type;

	return output;
}