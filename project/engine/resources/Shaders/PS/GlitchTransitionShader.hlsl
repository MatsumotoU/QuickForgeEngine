#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<GlitchOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

float Hash21(float32_t2 p)
{
    p = frac(p * float32_t2(123.34f, 456.21f));
    p += dot(p, p + 45.32f);
    return frac(p.x * p.y);
}

float BlockNoise(float32_t y, float32_t time)
{
    float32_t block = floor(y * 42.0f);
    float32_t tick = floor(time * 18.0f);
    return Hash21(float32_t2(block, tick)) * 2.0f - 1.0f;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t2 uv = input.texcoord.xy;
    float32_t intensity = saturate(gOffsetBuffer.parameters.x);

    // 強度が 0 のときは元画像をそのまま返して、遷移終了時のちらつきを防ぎます。
    if (intensity <= 0.001f)
    {
        output.color = gTexture.Sample(gSampler, uv);
        return output;
    }

    float32_t2 resolution = max(gOffsetBuffer.screenTime.xy, float32_t2(1.0f, 1.0f));
    float32_t time = gOffsetBuffer.screenTime.z;
    float32_t block = BlockNoise(uv.y, time);
    float32_t horizontalShift = block * gOffsetBuffer.parameters.z * intensity;

    // 細かい帯状のノイズと、時間で変化する粒状ノイズを重ねます。
    float32_t scanline = sin(uv.y * resolution.y * 1.7f + time * 35.0f) * 0.5f + 0.5f;
    float32_t grain = Hash21(floor(uv * resolution * 0.65f) + floor(time * 31.0f));
    float32_t noise = (grain - 0.5f) * gOffsetBuffer.parameters.y * intensity;

    float32_t colorShift = gOffsetBuffer.parameters.w * (0.5f + abs(block));
    float32_t2 shiftedUv = saturate(uv + float32_t2(horizontalShift, 0.0f));
    float32_t3 redSample = gTexture.Sample(gSampler, saturate(shiftedUv + float32_t2(colorShift, 0.0f))).rgb;
    float32_t3 baseSample = gTexture.Sample(gSampler, shiftedUv).rgb;
    float32_t3 blueSample = gTexture.Sample(gSampler, saturate(shiftedUv - float32_t2(colorShift, 0.0f))).rgb;
    float32_t3 glitched = float32_t3(redSample.r, baseSample.g, blueSample.b);

    glitched += noise;
    glitched *= 0.88f + scanline * 0.16f * intensity;
    // 色収差をわずかに加え、最大強度では画面全体が崩れるようにします。
    glitched = lerp(baseSample, glitched, saturate(0.65f + intensity * 0.55f));
    output.color = float32_t4(saturate(glitched), 1.0f);
    return output;
}
