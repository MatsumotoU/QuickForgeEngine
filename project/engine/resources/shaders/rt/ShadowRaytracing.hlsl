#include "../ShaderStructs/hlslTypeToCpp.h"

RaytracingAccelerationStructure g_scene : register(t0);
ConstantBuffer<CameraForGPU> g_camera : register(b0);

Texture2D<float4> g_position : register(t1);
Texture2D<float4> g_normal : register(t2);
Texture2D<float4> g_albedo : register(t3);
Texture2D<float4> g_material : register(t4);

RWTexture2D<float4> g_output : register(u0);

StructuredBuffer<uint3> g_globalTriIndices : register(t10);
StructuredBuffer<RaytracingVertexAttribute> g_globalVertexAttributes : register(t11);
struct InstanceMeta
{
    uint materialIndex;
    uint vertexBase;
    uint vertexCount;
    uint primitiveBase;
    float4 baseColor;
    row_major float4x4 uvTransform;
    float metallic;
    float smoothness;
    float2 padding;
};

StructuredBuffer<InstanceMeta> g_instanceMeta : register(t12);

Texture2D<float4> g_TextureArray[256] : register(t20);
SamplerState g_sampler : register(s0);

struct RayPayload
{
    float3 color;
    uint hit; // 0 = miss, 1 = hit
    uint rayType;
};

static const uint kShadowRay = 0;
static const uint kReflectionRay = 1;
static const float3 kLightDirection = float3(0.0f, -1.0f, 0.0f);
static const float3 kLightColor = float3(1.0f, 1.0f, 1.0f);
static const float kLightIntensity = 1.0f;
static const float kAmbientIntensity = 0.12f;

void ComputeHitAttributes(
    uint primGlobal,
    uint vertexBase,
    float2 bary,
    out float2 texcoord,
    out float3 objectNormal)
{
    uint3 tri = g_globalTriIndices[primGlobal];
    RaytracingVertexAttribute vertex0 = g_globalVertexAttributes[vertexBase + tri.x];
    RaytracingVertexAttribute vertex1 = g_globalVertexAttributes[vertexBase + tri.y];
    RaytracingVertexAttribute vertex2 = g_globalVertexAttributes[vertexBase + tri.z];

    float u = bary.x;
    float v = bary.y;
    float w = 1.0f - u - v;

    texcoord =
        vertex0.texcoord * w +
        vertex1.texcoord * u +
        vertex2.texcoord * v;
    objectNormal = normalize(
        vertex0.normal * w +
        vertex1.normal * u +
        vertex2.normal * v);
}

float3 EvaluateDirectLighting(float3 albedo, float3 worldNormal)
{
    float3 lightVector = normalize(-kLightDirection);
    float nDotL = saturate(dot(normalize(worldNormal), lightVector));
    float3 directLight = kLightColor * (nDotL * kLightIntensity);
    return albedo * (kAmbientIntensity + directLight);
}

float ComputeRayEpsilon(float3 worldPosition)
{
    float maxPosition = max(abs(worldPosition.x), max(abs(worldPosition.y), abs(worldPosition.z)));
    return max(0.0001f, maxPosition * 0.00001f);
}

float3 OffsetRayOrigin(float3 worldPosition, float3 worldNormal, float3 rayDirection)
{
    float normalSide = dot(worldNormal, rayDirection) >= 0.0f ? 1.0f : -1.0f;
    return worldPosition + worldNormal * normalSide * ComputeRayEpsilon(worldPosition);
}

[shader("raygeneration")]
void MyRayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;

    float4 rawPosition = g_position[launchIndex];
    float3 worldNormal = normalize(g_normal[launchIndex].xyz);
    float3 albedo = g_albedo[launchIndex].rgb;
    float albedpAlpha = g_albedo[launchIndex].a;

    if (rawPosition.w == 0.0f)
    {
        g_output[launchIndex] = float4(0.1f, 0.1f, 0.15f, 1.0f);
        return;
    }

    float3 worldPosition = rawPosition.xyz;
    float3 lightVector = normalize(-kLightDirection);
    float nDotL = saturate(dot(worldNormal, lightVector));

    // --- ① 直接見えている場所の影計算 ---
    RayPayload payload;
    payload.hit = 0;
    payload.rayType = kShadowRay;
    payload.color = float3(0.0f, 0.0f, 0.0f);

    // 光が表面側にある場合だけシャドウレイを飛ばす。
    if (nDotL > 0.0f)
    {
        RayDesc shadowRay;
        shadowRay.Origin = OffsetRayOrigin(worldPosition, worldNormal, lightVector);
        shadowRay.Direction = lightVector;
        shadowRay.TMin = 0.0f;
        shadowRay.TMax = 1000.0f;

        TraceRay(
            g_scene,
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE,
            0xFF, 0, 1, 0,
            shadowRay,
            payload);
    }

    float3 baseColor = albedo * kAmbientIntensity;
    if (payload.hit == 0)
    {
        baseColor = EvaluateDirectLighting(albedo, worldNormal);
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
        reflectRay.Origin = OffsetRayOrigin(worldPosition, worldNormal, reflectDir);
        reflectRay.Direction = reflectDir;
        reflectRay.TMin = 0.0f;
        reflectRay.TMax = 1000.0f;

        RayPayload reflectPayload;
        reflectPayload.hit = 0;
        reflectPayload.rayType = kReflectionRay;
        reflectPayload.color = float3(0.1f, 0.1f, 0.15f);

        TraceRay(g_scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, reflectRay, reflectPayload);

        // Fresnelを反射色への乗算ではなく、ベース色と反射色の混合率として使う。
        // これによりmetallic=1で「反射色 * 暗いalbedo」だけになる過度な暗化を防ぐ。
        float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
        float nDotV = saturate(dot(worldNormal, viewDir));
        float3 fresnel = f0 + (1.0f - f0) * pow(1.0f - nDotV, 5.0f);
        // 粗い反射の多点サンプリングは未実装なので、低 smoothness では
        // 鏡面成分を弱めつつ、金属の拡散成分を抑えてエネルギーを保存する。
        float smoothReflection = lerp(0.25f, 1.0f, saturate(smoothness));
        float3 reflectionWeight = saturate(fresnel * smoothReflection);
        float3 diffuseColor = baseColor * (1.0f - saturate(metallic));
        finalColor = diffuseColor * (1.0f - reflectionWeight)
            + reflectPayload.color * reflectionWeight;
    }

    g_output[launchIndex] = float4(finalColor, albedpAlpha);
}

// 2. ミスシェーダー
[shader("miss")]
void MyMiss(inout RayPayload payload : SV_RayPayload)
{
    payload.hit = 0;
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

    // シャドウレイは遮蔽の有無だけが必要なので、テクスチャと法線を評価しない。
    payload.hit = 1;
    if (payload.rayType == kShadowRay)
    {
        return;
    }

    uint primGlobal = primitiveBase + localPrim;

    float2 hitUV;
    float3 objectNormal;
    ComputeHitAttributes(
        primGlobal,
        vertexBase,
        attribs.barycentrics,
        hitUV,
        objectNormal);

    // 非一様スケールでも正しくなるよう、法線は WorldToObject の転置で変換する。
    float3 worldNormal = normalize(mul(objectNormal, (float3x3)WorldToObject3x4()));
    if (dot(worldNormal, WorldRayDirection()) > 0.0f)
    {
        worldNormal = -worldNormal;
    }

    float2 transformedUV = mul(float4(hitUV, 0.0f, 1.0f), meta.uvTransform).xy;
    float3 hitAlbedo =
        meta.baseColor.rgb *
        g_TextureArray[texID].SampleLevel(g_sampler, transformedUV, 0).rgb;

    // 反射像の中でも影が維持されるよう、反射ヒット点から光源への可視性を調べる。
    float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 lightVector = normalize(-kLightDirection);
    float nDotL = saturate(dot(worldNormal, lightVector));
    payload.color = hitAlbedo * kAmbientIntensity;

    if (nDotL > 0.0f)
    {
        RayDesc shadowRay;
        shadowRay.Origin = OffsetRayOrigin(hitPosition, worldNormal, lightVector);
        shadowRay.Direction = lightVector;
        shadowRay.TMin = 0.0f;
        shadowRay.TMax = 1000.0f;

        RayPayload shadowPayload;
        shadowPayload.color = float3(0.0f, 0.0f, 0.0f);
        shadowPayload.hit = 0;
        shadowPayload.rayType = kShadowRay;

        TraceRay(
            g_scene,
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE,
            0xFF, 0, 1, 0,
            shadowRay,
            shadowPayload);

        if (shadowPayload.hit == 0)
        {
            payload.color = EvaluateDirectLighting(hitAlbedo, worldNormal);
        }
    }
}
