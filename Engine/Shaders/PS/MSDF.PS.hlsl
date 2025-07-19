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

    // MSDFテクスチャをサンプリング
    float3 sampleColor = g_MsdfAtlas.Sample(g_Sampler, input.texcoord).rgb;

    // RGBチャンネルから単一の符号付き距離値を再構築
    float sd = median(sampleColor.r, sampleColor.g, sampleColor.b);
    float alpha = saturate(sd - 0.5 + 0.5 * g_Constants.DistanceRange);

    // 最終的な色を計算
    float4 finalColor = float4(input.color.rgb, input.color.a * alpha);

    // アルファがほぼ0のピクセルは破棄してパフォーマンス向上
    if (finalColor.a < 0.01f)
    {
        discard;
    }

    output.color = finalColor;
    return output;
}