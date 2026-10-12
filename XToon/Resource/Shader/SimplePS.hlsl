struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPos : WORLD_POS;
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
	float4 Directional;
	float4 CameraPos;
	float Exponent;
	bool UseHighlight;
};

Texture2D XToonTexture : register(t0);
SamplerState XToonTextureSample : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    float3 N = normalize(input.Normal);
	float3 L = normalize(-Directional.xyz);
	float3 V = normalize(CameraPos.xyz - input.WorldPos.xyz);
	float3 R = reflect(-L, N);

	float NoL = saturate(dot(N,L));
	float NoV = saturate(dot(N,V)); // for silhouette
	float VoR = saturate(dot(V,R)); // for highlight
	
	float silhouette = 1.0f - saturate(pow(NoV, Exponent));
	float highlight = 1.0f - saturate(pow(VoR, Exponent));
	float v = lerp(silhouette, highlight, UseHighlight);

	float2 uv = float2(NoL, v);
	output.Color.rgb = XToonTexture.Sample(XToonTextureSample, uv).rgb;
    return output;
}