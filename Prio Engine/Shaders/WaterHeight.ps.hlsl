struct PS_WATER_INPUT
{
	float4 ProjPos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 UV : TEXCOORD0;
};

float WaterHeightPS( PS_WATER_INPUT input ) : SV_TARGET
{
	return input.WorldPos.y;
}