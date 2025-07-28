#include "../ShaderStructs/Object3d.hlsli"

struct ColorCorrectionOffset
{
    float exposure; // 露出
    float contrast; // コントラスト
    float saturation; // 彩度
    float gamma; // ガンマ
    float hue; // 色相
};

ConstantBuffer<ColorCorrectionOffset> gOffsetBuffer : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

// 彩度調整
float3 AdjustSaturation(float3 color, float saturation)
{
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    return lerp(float3(gray, gray, gray), color, saturation);
}

// RGB→HSV変換
float3 RGBtoHSV(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = c.g < c.b ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
    float4 q = c.r < p.x ? float4(p.xyw, c.r) : float4(c.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    float h = abs(q.z + (q.w - q.y) / (6.0 * d + e));
    float s = d / (q.x + e);
    float v = q.x;
    return float3(h, s, v);
}

// HSV→RGB変換
float3 HSVtoRGB(float3 hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float3 rgb = float3(0.0, 0.0, 0.0);

    if (s == 0.0)
    {
        rgb = float3(v, v, v);
    }
    else
    {
        h = frac(h) * 6.0;
        int i = (int) floor(h);
        float f = h - i;
        float p = v * (1.0 - s);
        float q = v * (1.0 - s * f);
        float t = v * (1.0 - s * (1.0 - f));
        if (i == 0)
            rgb = float3(v, t, p);
        else if (i == 1)
            rgb = float3(q, v, p);
        else if (i == 2)
            rgb = float3(p, v, t);
        else if (i == 3)
            rgb = float3(p, q, v);
        else if (i == 4)
            rgb = float3(t, p, v);
        else
            rgb = float3(v, p, q);
    }
    return rgb;
}

// 色相調整
float3 AdjustHue(float3 color, float hue)
{
    float3 hsv = RGBtoHSV(color);
    // hueは度数なので360で割って正規化
    hsv.x = frac(hsv.x + hue / 360.0);
    return HSVtoRGB(hsv);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);

    // 露出
    textureColor.rgb *= pow(2.0, gOffsetBuffer.exposure);
    // コントラスト
    textureColor.rgb = (textureColor.rgb - 0.5) * gOffsetBuffer.contrast + 0.5;
    // 彩度
    textureColor.rgb = AdjustSaturation(textureColor.rgb, gOffsetBuffer.saturation);
    // 色相
    textureColor.rgb = AdjustHue(textureColor.rgb, gOffsetBuffer.hue);
    // ガンマ補正
    textureColor.rgb = pow(textureColor.rgb, float3(1.0 / gOffsetBuffer.gamma, 1.0 / gOffsetBuffer.gamma, 1.0 / gOffsetBuffer.gamma));
    
    output.color = textureColor;
    return output;
}