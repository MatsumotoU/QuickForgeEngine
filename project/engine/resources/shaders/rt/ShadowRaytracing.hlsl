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

    // 1. 各種GBufferの情報を取得
    float4 rawPosition = g_position[launchIndex];
    float3 worldNormal = g_normal[launchIndex].xyz;
    float3 albedo = g_albedo[launchIndex].rgb;
    float albedpAlpha = g_albedo[launchIndex].a;

    // 2. 何もなければアルベドをそのまま出力する
    if (rawPosition.w == 0.0f)
    {
        g_output[launchIndex] = float4(1.0f, 0.0f, 1.0f, 1.0f);
        return;
    }
    float3 worldPosition = rawPosition.xyz;

    // 3. ライティング・光源の設定（仮の平行光源）
    float3 lightDir = normalize(float3(0.0f, -1.0f, 0.0f));
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float3 ambient = albedo * 0.2f; // 環境光（影の部分の暗さ）

    // 4. シャドウレイの構築
    RayDesc ray;
    // 始点はGBufferの位置。自分のポリゴンに誤衝突するのを防ぐため、法線方向に少し浮かせます（バイアス）
    ray.Origin = worldPosition + worldNormal * 0.05f;
    ray.Direction = -lightDir; // 光源への方向
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f; // 点光源なら光源までの距離

    // ペイロードの初期化（最初は「影に当たっていない」とする）
    ShadowPayload payload;
    payload.isHit = false;

    // 5. レイを飛ばす
    // 影バッファ専用のフラグを設定して高速化
    TraceRay(
    g_scene,
    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
    RAY_FLAG_FORCE_OPAQUE |
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
    0xFF, 0, 1, 0, ray, payload
);

    // 6. ライティングの計算
    float3 finalColor = albedo;
    if (payload.isHit){
        // 影にヒットしたら、環境光にする
        finalColor = albedo * 0.15f;
    }
    else
    {
        // 光が当たっている場所は、簡単なライティングを計算
        float dotNL = saturate(dot(worldNormal, -lightDir));
        finalColor = albedo * (dotNL * 0.8f + 0.2f);
    }

    // 最終出力を書き込む
    g_output[launchIndex] = float4(finalColor, albedpAlpha);
}

// 2. ミスシェーダー
[shader("miss")]
void MyMiss(inout ShadowPayload payload : SV_RayPayload)
{
    // 何にも当たらなかったので影ではない
    payload.isHit = false;
}
// 3. クローストヒットシェーダー
[shader("closesthit")]
void MyClosestHit(inout ShadowPayload payload : SV_RayPayload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.isHit = true;
}