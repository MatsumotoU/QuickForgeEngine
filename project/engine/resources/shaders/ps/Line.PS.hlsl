struct PixelShaderInput
{
	float4 position : SV_Position;
    float4 color : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET0
{
    return input.color;
}
