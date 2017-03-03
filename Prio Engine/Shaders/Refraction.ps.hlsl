Texture2D refractionTex;
SamplerState SampleType;

cbuffer LightBuffer
{
	float4 ambientColour;
	float4 diffuseColour;
	float3 lightDirection;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float clip : SV_ClipDistance0;
};

float4 RefractionPS(PixelInputType input) : SV_TARGET
{
	float4 texColour = refractionTex.Sample(SampleType, input.tex);

	float4 colour = ambientColour;
	float3 lightDir = -lightDirection;

	float lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		colour += (diffuseColour * lightIntensity);
	}

	colour = saturate(colour);

	colour = colour * texColour;

	return colour;
}