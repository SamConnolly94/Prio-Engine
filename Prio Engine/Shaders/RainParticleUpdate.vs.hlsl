struct VertexInputType
{
	float3 InitialPosition	: POSITION;
	float3 InitialVelocity  : VELOCITY;
	float2 Size				: SIZE;
	float  Age				: AGE;
	unsigned int  Type		: TYPE;
};

VertexInputType RainUpdateVS(VertexInputType input)
{
	return input;
}
