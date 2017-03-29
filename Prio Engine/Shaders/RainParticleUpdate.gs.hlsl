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

struct ParticleType
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
	uint   Type			  : TYPE;
};

Texture1D randomTexture;
SamplerState sampleType;

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (GameTime + offset);

	// coordinates in [-1,1]
	float3 v = randomTexture.SampleLevel(sampleType, u, 0).xyz;

	return v;
}

//////////////////////////
// Geometry Shader
/////////////////////////

[maxvertexcount(6)]
void RainUpdateGS(point ParticleType input[1], inout PointStream<ParticleType> outStream)
{
	input[0].Age += FrameTime;

	if (input[0].Type == PT_EMITTER)
	{
		if (input[0].Age > 0.001f)
		{
			for (int i = 0; i < 6; i++)
			{
				float3 randomVec = 35.0f * RandVec3( (float)i / 5.0f);
				randomVec.y = 40.0f;

				ParticleType p;
				p.InitialPosition = EmitPos + randomVec;
				p.InitialVelocity = float3(0.0f, 0.0f, 0.0f);
				p.Size = float2(1.0f, 1.0f);
				p.Age = 0.0f;
				p.Type = PT_FLARE;

				outStream.Append(p);
			}
			// Reset the age to 0.
			input[0].Age = 0.0f;
		}
		outStream.Append(input[0]);
	}
	else
	{
		if (input[0].Age <= 3.0f)
		{
			outStream.Append(input[0]);
		}
		else
		{
			float3 vRandom = 35.0f * RandVec3((float)5.0f);
			vRandom.y = 40.0f;
			input[0].InitialPosition = EmitPos.xyz + vRandom;
			input[0].Age = 0.0f;
			outStream.Append(input[0]);
		}
	}
}
