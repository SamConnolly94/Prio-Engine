#define PT_EMITTER 0
#define PT_FLARE 1

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

struct GeometryInputType
{
	float3 WorldPosition  : POSITION;
	unsigned int Type	  : TYPE;
};

struct PixelInputType
{
	float4 ProjectedPosition : SV_POSITION;
	float2 UV				 : TEXCOORD0;
};

[maxvertexcount(2)]
void RainGS(point GeometryInputType input[1], inout LineStream<PixelInputType> lineStream)
{
	float Acceleration = float3(0.0f, -9.8f, 0.0f);

	if (input[0].Type != PT_EMITTER)
	{
		float3 p0 = input[0].WorldPosition.xyz;
		float3 p1 = input[0].WorldPosition.xyz + 0.07f * Acceleration;

		PixelInputType output1;
		matrix viewProj = ViewMatrix * ProjMatrix;
		output1.ProjectedPosition = mul(float4(p0, 1.0f), viewProj);
		output1.UV = float2(0.0f, 0.0f);

		lineStream.Append(output1);

		PixelInputType output2;
		output2.ProjectedPosition = mul(float4(p1, 1.0f), viewProj);
		output2.UV = float2(1.0f, 1.0f);

		lineStream.Append(output2);

	}
}
