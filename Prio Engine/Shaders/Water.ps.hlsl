SamplerState SampleType;

Texture2D reflectionMap;
Texture2D refractionMap;
Texture2D normalMap;

cbuffer WaterBuffer
{
	float waterTranslation;
	float reflectRefractRatio;
	float2 waterBufferPadding;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 reflectionPos : TEXCOORD1;
	float4 refractionPos : TEXCOORD2;
};

float4 WaterPS(PixelInputType input) : SV_TARGET
{
	float blend = 0.6f;

	// Add the distance which the water is to be moved by to the pos on the screen.
	input.tex.y += waterTranslation;

	float2 reflectTexCoord;
	float2 refractionTexCoord;

	// Covert tex coordinates into range -1 to + 1.
	reflectTexCoord.x = input.reflectionPos.x / input.reflectionPos.w / 2.0f + 0.5f;
	reflectTexCoord.y = -input.reflectionPos.y / input.reflectionPos.w / 2.0f + 0.5f;
	refractionTexCoord.x = input.refractionPos.x / input.refractionPos.w / 2.0f + 0.5f;
	refractionTexCoord.y = -input.refractionPos.y / input.refractionPos.w / 2.0f + 0.5f;

	// Sample the normal at the current UV coordinates passed in in the pixel input type.
	float4 normalSample = normalMap.Sample(SampleType, input.tex);
	// Convert the sampled normal map into range -1 to 1.
	float3 normal = (normalSample.xyz * 2.0f) - 1.0f;

	// Apply a rippling effect to the water by distorting it.
	reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractRatio);
	refractionTexCoord = refractionTexCoord + (normal.xy * reflectRefractRatio);

	float4 reflectionColour = reflectionMap.Sample(SampleType, reflectTexCoord);
	float4 refractionColour = refractionMap.Sample(SampleType, refractionTexCoord);

	float4 colour = lerp(reflectionColour, refractionColour, blend);

	return colour;
}