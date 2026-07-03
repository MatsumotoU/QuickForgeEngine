RaytracingAccelerationStructure g_scene : register(t0);
RWTexture2D<float4> g_output : register(u0);

Texture2D<float4> g_position : register(t1);
Texture2D<float4> g_normal : register(t2);
Texture2D<float4> g_albedo : register(t3);

// ペイロード（光線が持ち運ぶデータの箱）の定義
struct ShadowPayload
{
    bool isHit;
};

[shader("raygeneration")]
void MyRayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;

    // 1. 💡 画面のピクセル位置に対応するGBufferの情報を取得
    float4 rawPosition = g_position[launchIndex];
    float3 worldNormal = g_normal[launchIndex].xyz;
    float3 albedo = g_albedo[launchIndex].rgb;
    float albedpAlpha = g_albedo[launchIndex].a;

    // 2. 何もなければアルベドをそのまま出力する
    if (rawPosition.w == 0.0f)
    {
        g_output[launchIndex] = float4(albedo, albedpAlpha);
        return;
    }
    float3 worldPosition = rawPosition.xyz;

    // 3. ライティング・光源の設定（仮の平行光源）
    float3 lightDir = normalize(float3(0.5f, 1.0f, -0.5f));
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float3 ambient = albedo * 0.2f; // 環境光（影の部分の暗さ）

    // 4. 💡 シャドウレイ（影用光線）の構築
    RayDesc ray;
    // 始点はGBufferの位置。自分のポリゴンに誤衝突するのを防ぐため、法線方向に少し浮かせます（バイアス）
    ray.Origin = worldPosition + worldNormal * 0.001f;
    ray.Direction = lightDir; // 光源への方向
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f; // 点光源なら光源までの距離

    // ペイロードの初期化（最初は「影に当たっていない」とする）
    ShadowPayload payload;
    payload.isHit = false;

    // 5. 💡 レイを飛ばす
    // 影バッファ専用のフラグを設定して高速化
    TraceRay(
        g_scene,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE,
        0xFF, 0, 1, 0, ray, payload
    );

    // 6. 💡 最終的な色の計算
    float3 finalColor = ambient;

    if (!payload.isHit)
    {
        // 影に遮られていなければ、通常のディフューズ（拡散反射）を計算
        float ndl = max(0.0f, dot(worldNormal, lightDir));
        finalColor += albedo * lightColor * ndl;
    }

    // 出力
    g_output[launchIndex] = float4(finalColor, albedpAlpha);
}

// 2. ミスシェーダー（外れたときの救済）
[shader("miss")]
// 🌟 2. 引数の型を自作した構造体（RayPayload）にする！
void MyMiss(inout ShadowPayload payload : SV_RayPayload)
{
    // 何にも当たらなかったので影ではない
    payload.isHit = false;
}

[shader("closesthit")]
void MyClosestHit(inout ShadowPayload payload : SV_RayPayload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.isHit = true;
}