struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

Texture2D SphereMap : register(t0);
SamplerState SphereMapSample : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    float3 N = normalize(input.Normal);
	float2 uv = 0.5f + 0.5f * N.xy;
	output.Color.rgb = SphereMap.Sample(SphereMapSample, uv).rgb;

    return output;
}