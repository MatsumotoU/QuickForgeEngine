RWTexture2D<float4> g_output : register(u0);

struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    
    // テスト用に単色を出力
    g_output[launchIndex] = float4(0.1f, 0.2f, 0.4f, 1.0f);
}

// 2. ミスシェーダー（外れたときの救済）
[shader("miss")]
// 🌟 2. 引数の型を自作した構造体（RayPayload）にする！
void MyMiss(inout RayPayload payload : SV_RayPayload)
{
    // 構造体のメンバ変数に色を代入する
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
}