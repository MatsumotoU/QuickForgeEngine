#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

static const float ditherMatrix[4][4] =
{
    { 0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0 },
    { 12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0 },
    { 3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0 },
    { 15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0 }
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<EchoSphereInfo> gEchoSphereInfo : register(b2);
Texture2D<float32_t4> gTexture : register(t0);
StructuredBuffer<EchoSphere> gEchoSpheres : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float4 color = gMaterial.color * textureColor;
    float alpha = gMaterial.color.a * textureColor.a;
    if (alpha == 0.0f)
    {
        discard;
    }
    
    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = alpha;
    }
    
    if (gMaterial.enableLighting)
    {
        bool shouldDraw = false;
        for (int i = 0; i < gEchoSphereInfo.count; i++)
        {
            if (gEchoSpheres[i].isActive)
            {
                float dist = length(input.worldPosition - gEchoSpheres[i].sphereCenter);
                float edgeMin = gEchoSpheres[i].sphereRadius - gEchoSpheres[i].sphereThickness * 0.5f;
                float edgeMax = gEchoSpheres[i].sphereRadius + gEchoSpheres[i].sphereThickness * 0.5f;
                bool isOnEdge = (dist >= edgeMin) && (dist <= edgeMax);

                if (isOnEdge)
                {
                    shouldDraw = true;
                    break;
                }
            }
        }
        if (!shouldDraw)
        {
            discard;
        }
    }

    output.color = color;
    return output;
}
