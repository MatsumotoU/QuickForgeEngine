RaytracingAccelerationStructure g_scene : register(t0);
RWTexture2D<float4> g_output : register(u0);

Texture2D<float4> g_position : register(t1);
Texture2D<float4> g_normal : register(t2);
Texture2D<float4> g_albedo : register(t3);
Texture2D<float4> g_material : register(t4);

struct CameraData
{
    float3 position;
};
ConstantBuffer<CameraData> g_camera : register(b0);

// 【変更】ひとつのペイロードに役割をまとめます
struct ShadowPayload
{
    bool isHit; // 衝突したかどうか
    bool isReflection; // true: 反射レイとして飛ばしている / false: シャドウレイとして飛ばしている
    float3 color; // 反射レイの時に、結果の色を格納する場所
};

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

    ShadowPayload payload;
    payload.isHit = false;
    payload.isReflection = false; // これはシャドウ用のレイ
    payload.color = float3(0.0f, 0.0f, 0.0f);

    TraceRay(
        g_scene,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF, 0, 1, 0,
        shadowRay,
        payload);

    float3 baseColor = albedo;
    if (payload.isHit)
    {
        baseColor = albedo * 0.15f;
    }
    else
    {
        float dotNL = saturate(dot(worldNormal, -lightDir));
        baseColor = albedo * (dotNL * 0.8f + 0.2f);
    }

    // --- ② 【新規】既存のTraceRayを使い回してスペキュラー（反射）を計算 ---
    float3 viewDir = normalize(worldPosition - g_camera.position);
    float3 reflectDir = reflect(viewDir, worldNormal);

    RayDesc reflectRay;
    reflectRay.Origin = worldPosition + worldNormal * 0.05f;
    reflectRay.Direction = reflectDir;
    reflectRay.TMin = 0.001f;
    reflectRay.TMax = 1000.0f;

    ShadowPayload reflectPayload;
    reflectPayload.isHit = false;
    reflectPayload.isReflection = true; // 💡「これは反射レイだよ」と教えてあげる
    reflectPayload.color = float3(0.1f, 0.1f, 0.15f); // 背景色を初期値にしておく

    // 既存のヒットグループ・ミスシェーダーをそのまま呼び出す
    TraceRay(g_scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, reflectRay, reflectPayload);

    // --- ③ 元の色と反射した色を合成 ---
    float specularIntensity = 0.4f;
    float3 finalColor = lerp(baseColor, reflectPayload.color, specularIntensity);

    g_output[launchIndex] = float4(finalColor, albedpAlpha);
}

// 2. ミスシェーダー
[shader("miss")]
void MyMiss(inout ShadowPayload payload : SV_RayPayload)
{
    // 影レイなら既存通り。反射レイなら何もしない（初期値の背景色のままになる）
    payload.isHit = false;
}

// 3. クローストヒットシェーダー
[shader("closesthit")]
void MyClosestHit(inout ShadowPayload payload : SV_RayPayload, BuiltInTriangleIntersectionAttributes attribs)
{
    // 💡 自分がどっちの目的で呼び出されたかで処理を分岐する！
    if (payload.isReflection)
    {
        // 反射レイが何かに当たった場合：鏡の奥の世界

        // 鏡の奥で当たった位置を計算
        float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        float3 hitNormal = float3(0.0f, 1.0f, 0.0f); // 仮の法線
        float3 hitAlbedo = float3(0.7f, 0.7f, 0.7f); // 仮の色
        float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));

        // 鏡の奥から、さらに「純粋な影レイ」を飛ばす！
        RayDesc shadowRay;
        shadowRay.Origin = hitPosition + hitNormal * 0.05f;
        shadowRay.Direction = -lightDir;
        shadowRay.TMin = 0.001f;
        shadowRay.TMax = 1000.0f;

        ShadowPayload innerShadowPayload;
        innerShadowPayload.isHit = false;
        innerShadowPayload.isReflection = false; // ここは純粋な影チェック
        innerShadowPayload.color = float3(0.0f, 0.0f, 0.0f);

        // 自分自身の所属するパイプラインを再帰的に呼び出す
        TraceRay(
            g_scene,
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
            0xFF, 0, 1, 0,
            shadowRay,
            innerShadowPayload);

        // 鏡の奥のライティング結果を、呼び出し元の color に書き戻す
        if (innerShadowPayload.isHit)
        {
            payload.color = hitAlbedo * 0.15f; // 鏡の奥も影だった
        }
        else
        {
            float dotNL = saturate(dot(hitNormal, -lightDir));
            payload.color = hitAlbedo * (dotNL * 0.8f + 0.2f);
        }
    }
    else
    {
        // 💡 既存の挙動：純粋な影レイとして当たった場合は、単に true にするだけ
        payload.isHit = true;
    }
}