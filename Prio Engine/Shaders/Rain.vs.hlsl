//////////////////////////
// Constant Buffer
/////////////////////////

cbuffer MatrixBuffer : register(b0)
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjMatrix;
}

cbuffer FrameBuffer : register(b1)
{
	float3 CameraPos;
	float3 EmitPos;
	float3 EmitDir;
	float GameTime;
	float FrameTime;
	float frameBufferPadding;
}
//////////////////////////
// Input Structures
/////////////////////////

struct VertexInputType
{
	float3 InitialPosition	: POSITION;
	float3 InitialVelocity  : VELOCITY;
	float2 Size				: SIZE;
	float Age				: AGE;
	uint Type				: TYPE;
};

struct GeometryInputType
{
	float3 WorldPosition  : POSITION;
	unsigned int Type	  : TYPE;
};

//////////////////////////
// Vertex Shader
/////////////////////////

GeometryInputType RainVS(VertexInputType input)
{
	GeometryInputType output;

	float3 Acceleration = float3(0.0f, -9.8f, 0.0f);

	// Acceleration due to gravity equation.
	output.WorldPosition = 0.5f * input.Age * input.Age * Acceleration + input.Age + input.InitialVelocity + input.InitialPosition;

	output.Type = input.Type;

	return output;
}
