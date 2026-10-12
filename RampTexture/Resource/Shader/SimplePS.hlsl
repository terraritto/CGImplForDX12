struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

cbuffer Scene : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
	float3 Directional;
};

Texture2D RampTexture : register(t0);
SamplerState RampTextureSample : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    float3 N = normalize(input.Normal);
	float3 L = normalize(-Directional);
	float NoL = saturate(dot(N,L));
	float2 uv = float2(NoL, 0);
	output.Color.rgb = RampTexture.Sample(RampTextureSample, uv).rgb;

    return output;
}