#include "../ShaderStructs/hlslTypeToCpp.h"

RaytracingAccelerationStructure g_scene : register(t0);
ConstantBuffer<CameraForGPU> g_camera : register(b0);

Texture2D<float4> g_position : register(t1);
Texture2D<float4> g_normal : register(t2);
Texture2D<float4> g_albedo : register(t3);
Texture2D<float4> g_material : register(t4);

RWTexture2D<float4> g_output : register(u0);

StructuredBuffer<uint3> g_globalTriIndices : register(t10);
StructuredBuffer<float2> g_globalVertexUVs : register(t11);
struct InstanceMeta
{
    uint materialIndex;
    uint vertexBase;
    uint vertexCount;
    uint primitiveBase;
};

StructuredBuffer<InstanceMeta> g_instanceMeta : register(t12);

Texture2D<float4> g_TextureArray[256] : register(t20);
SamplerState g_sampler : register(s0);

struct RayPayload
{
    float3 color;
    float2 debugUV; // デバッグ用 UV (R=U, G=V)
    uint hit; // 0 = miss, 1 = hit
};

float2 ComputeHitUV_Global(uint primGlobal, uint vertexBase, float2 bary)
{
    uint3 tri = g_globalTriIndices[primGlobal];
    float2 uv0 = g_globalVertexUVs[vertexBase + tri.x];
    float2 uv1 = g_globalVertexUVs[vertexBase + tri.y];
    float2 uv2 = g_globalVertexUVs[vertexBase + tri.z];

    float u = bary.x;
    float v = bary.y;
    float w = 1.0f - u - v;
    return uv0 * w + uv1 * u + uv2 * v;
}

[shader("raygeneration")]
void MyRayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;

    float4 rawPosition = g_position[launchIndex];
    float3 worldNormal = g_normal[launchIndex].xyz;
    float3 albedo = g_albedo[launchIndex].rgb;
    float albedpAlpha = g_albedo[launchIndex].a;

    if (rawPosition.w == 0.0f)
    {
        g_output[launchIndex] = float4(0.1f, 0.1f, 0.15f, 1.0f);
        return;
    }

    float3 worldPosition = rawPosition.xyz;
    float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));

    // --- ① 直接見えている場所の影計算 ---
    RayDesc shadowRay;
    shadowRay.Origin = worldPosition + worldNormal * 0.05f;
    shadowRay.Direction = -lightDir;
    shadowRay.TMin = 0.001f;
    shadowRay.TMax = 1000.0f;

    // ペイロード型を統一 (RayPayload)
    RayPayload payload;
    payload.hit = 0;
    payload.debugUV = float2(0.0f, 0.0f);
    payload.color = float3(0.0f, 0.0f, 0.0f);
    payload.hit = 0;
    payload.debugUV = float2(0.0f, 0.0f);

    TraceRay(
        g_scene,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF, 0, 1, 0,
        shadowRay,
        payload);

    float3 baseColor = albedo;
    if (payload.hit != 0)
    {
        baseColor = albedo * 0.15f;
    }
    else
    {
        float dotNL = saturate(dot(worldNormal, -lightDir));
        baseColor = albedo * (dotNL * 0.8f + 0.2f);
    }

    // --- ② 反射計算 ---
    float4 materialData = g_material[launchIndex];
    float metallic = materialData.r;
    float smoothness = materialData.g;

    float3 finalColor = baseColor;

    if (smoothness > 0.1f)
    {
        // viewDir はカメラ位置 - サーフェス位置（サーフェスからカメラへのベクトル）
        float3 viewDir = normalize(g_camera.cameraPosition - worldPosition);
        float3 reflectDir = reflect(-viewDir, worldNormal); // incident = -viewDir (視線は surface->camera)

        RayDesc reflectRay;
        reflectRay.Origin = worldPosition + worldNormal * 0.05f;
        reflectRay.Direction = reflectDir;
        reflectRay.TMin = 0.001f;
        reflectRay.TMax = 1000.0f;

        RayPayload reflectPayload;
        reflectPayload.hit = 0;
        reflectPayload.debugUV = float2(0.0f, 0.0f);
        reflectPayload.color = float3(0.1f, 0.1f, 0.15f);

        TraceRay(g_scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, reflectRay, reflectPayload);

        // Fresnelを反射色への乗算ではなく、ベース色と反射色の混合率として使う。
        // これによりmetallic=1で「反射色 * 暗いalbedo」だけになる過度な暗化を防ぐ。
        float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
        float nDotV = saturate(dot(worldNormal, viewDir));
        float3 fresnel = f0 + (1.0f - f0) * pow(1.0f - nDotV, 5.0f);
        float3 reflectionWeight = saturate(fresnel * smoothness);
        finalColor = lerp(baseColor, reflectPayload.color, reflectionWeight);
    }

    g_output[launchIndex] = float4(finalColor, albedpAlpha);
}

// 2. ミスシェーダー
[shader("miss")]
void MyMiss(inout RayPayload payload : SV_RayPayload)
{
    payload.hit = 0;
    payload.debugUV = float2(0.0f, 0.0f);
    payload.color = float3(0.1f, 0.1f, 0.15f);
}

// 3. クローストヒットシェーダー
[shader("closesthit")]
void MyClosestHit(inout RayPayload payload : SV_RayPayload, BuiltInTriangleIntersectionAttributes attribs)
{
    uint localPrim = PrimitiveIndex();
    uint instId = InstanceID();

    InstanceMeta meta = g_instanceMeta[instId];
	uint texID = meta.materialIndex;
	uint vertexBase = meta.vertexBase;
	uint primitiveBase = meta.primitiveBase;

    uint primGlobal = primitiveBase + localPrim;

    float2 hitUV = ComputeHitUV_Global(primGlobal, vertexBase, attribs.barycentrics);

    // デバッグ: UV を payload に入れる
    payload.debugUV = hitUV;
    payload.hit = 1;

    // UV の可視化として color に入れておく（R=U, G=V）
    payload.color = g_TextureArray[texID].SampleLevel(g_sampler, hitUV, 0).rgb;
}
