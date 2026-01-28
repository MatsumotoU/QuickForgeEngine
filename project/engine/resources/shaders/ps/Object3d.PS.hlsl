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
ConstantBuffer<CameraForGPU> gCamera : register(b2);
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

    if (alpha == 0.0f)
    {
        discard;
    }

    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -normalize(gDirectionalLight.direction));
        NdotL = max(NdotL, 0.0f);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = alpha;
        
        float32_t3 toEye = normalize(gCamera.cameraPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        float32_t3 halfVector = normalize(-normalize(gDirectionalLight.direction) + toEye);
        float32_t NDotH = dot(normalize(input.normal), halfVector);
        float32_t specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        
        float32_t3 diffuse =
             gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        float32_t3 specular = 
            gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
        output.color.rgb = diffuse + specular;
        output.color.a = alpha;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }

    return output;
}
