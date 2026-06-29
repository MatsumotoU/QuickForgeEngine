RaytracingAccelerationStructure g_scene : register(t0);
RWTexture2D<float4> g_output : register(u0);

// ペイロード（光線が持ち運ぶデータの箱）の定義
struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    // 1. スクリーン座標（0.0～1.0）の計算
    float2 d = (((float2) launchIndex + 0.5f) / (float2) launchDim) * 2.f - 1.f;
    d.y = -d.y; // 画面の上をプラス方向にする

    // 2. 超簡易カメラ：レイの「始点」と「進む方向」を決める
    // カメラは (0, 0, -2) にあり、前方の平行な光線を飛ばすイメージ
    RayDesc ray;
    // レイの「始点」を画面のピクセル位置(d.x, d.y)ではなく、
    // 原点 (0, 0, -2) という1点（点カメラ）に固定する
    ray.Origin = float3(0.0f, 0.0f, -2.0f);

    // 光線の「方向」を、中心から外側に向けて扇状に広がるように飛ばす（ピンホールカメラ化）
    // これにより、視野が広がって三角形を捉えやすくなります
    ray.Direction = normalize(float3(d.x, d.y, 1.0f));
    
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    // 3. 光線に持たせるデータの初期化（外れたらこの色になるようにしておく）
    RayPayload payload;
    // 1. 初期値を「真っ黒」にしておく
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    TraceRay(g_scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, ray, payload);

    // 出力
    g_output[launchIndex] = payload.color;
}

// 2. ミスシェーダー（外れたときの救済）
[shader("miss")]
// 🌟 2. 引数の型を自作した構造体（RayPayload）にする！
void MyMiss(inout RayPayload payload : SV_RayPayload)
{
    // 構造体のメンバ変数に色を代入する
    payload.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
}

[shader("closesthit")]
void MyClosestHit(inout RayPayload payload : SV_RayPayload, BuiltInTriangleIntersectionAttributes attribs)
{
    // 三角形に当たったら、お祝いの「赤色」にする！
    payload.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
}