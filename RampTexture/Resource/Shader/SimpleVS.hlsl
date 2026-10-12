struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

cbuffer Scene : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
	float3 Directional;
};

VSOutput main(const VSInput input)
{
	VSOutput output = (VSOutput)0;

	// pos
	float4 localPosition = float4(input.Position, 1.0f);
	float4 worldPosition = mul(World, localPosition);
	float4 viewPosition = mul(View, worldPosition);
	float4 projPosition = mul(Proj, viewPosition);
	output.Position = projPosition;

	// normal
	output.Normal   = normalize(mul((float3x3)World, input.Normal));
	
	return output;
}