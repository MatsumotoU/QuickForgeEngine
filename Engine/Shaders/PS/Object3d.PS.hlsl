#include "../ShaderStructs/Object3d.hlsli"

// 4x4 Bayerマトリクス
static const float ditherMatrix[4][4] =
{
    { 0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0 },
    { 12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0 },
    { 3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0 },
    { 15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0 }
};

struct DirectionalLight
{
    float32_t4 color;// ライトの色
    float32_t3 direction;// ライトの向き
    float intensity;// 輝度
};

struct Material{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float alpha = gMaterial.color.a * textureColor.a;

    // 透明度が0なら必ず消す
    if (alpha == 0.0f)
    {
        discard;
    }

    // alphaが小さいほど間隔が広がる（粗くなる）
    float scale = lerp(4.0f, 1.0f, alpha); // 4.0fは最大粗さ（調整可）
    int2 pixelPos = (int2(input.position.xy / scale)) & 3;
    float ditherThreshold = ditherMatrix[pixelPos.y][pixelPos.x];

    float threshold = lerp(0.0f, 1.0f, 1.0f - alpha) * ditherThreshold;

    if (alpha < threshold)
    {
        discard;
    }

    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = 1.0f;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }

    return output;
}