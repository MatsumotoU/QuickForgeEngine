#include "../ShaderStructs/Font.hlsli"

struct Constants
{
    float32_t2 AtlasSize; // アトラスのピクセルサイズ (width, height)
    float32_t DistanceRange; // JSONから読み込んだ distanceRange (pixelRange)
    float32_t padding;
};

ConstantBuffer<Constants> g_Constants : register(b0);
Texture2D<float4> g_MsdfAtlas : register(t0);
SamplerState g_Sampler : register(s0);

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float3 sampleColor = g_MsdfAtlas.Sample(g_Sampler, input.texcoord).rgb;
    float sd = median(sampleColor.r, sampleColor.g, sampleColor.b);

    // アンチエイリアス処理
    float edgeWidth = fwidth(sd) * 0.1;
    float alpha = smoothstep(0.5 - edgeWidth, 0.5 + edgeWidth, sd);

    float4 finalColor = float4(input.color.rgb, input.color.a * alpha);

    if (finalColor.a < 0.01f)
    {
        discard;
    }

    output.color = finalColor;
    return output;
}