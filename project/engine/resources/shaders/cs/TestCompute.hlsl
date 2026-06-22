RWTexture2D<float4> g_outputTex : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // テクスチャのサイズを取得
    uint width, height;
    g_outputTex.GetDimensions(width, height);

    // 画面外のピクセルなら処理しない（安全対策）
    if (DTid.x >= width || DTid.y >= height)
        return;

    // 座標に応じたグラデーション色を計算 (0.0 ～ 1.0)
    float2 uv = (float2) DTid.xy / float2(width, height);
    float4 finalColor = float4(uv.x, uv.y, 0.0f, 1.0f); // 黄色〜赤〜緑のグラデーション

    // UAVテクスチャの指定したピクセルに直接書き込む！
    g_outputTex[DTid.xy] = finalColor;
}