RWTexture2D<float4> g_outputTex : register(u0);

// レイ（光線）の構造体
struct Ray
{
    float3 origin; // スタート地点（カメラの位置）
    float3 direction; // 進む方向
};

// 球体の構造体
struct Sphere
{
    float3 center; // 中心座標
    float3 color; // 球体の色
    float radius; // 半径
};

// レイと球体の当たり判定（交差判定）関数
bool HitSphere(Ray ray, Sphere sphere, out float t, out float3 normal)
{
    // 最初にあらかじめ初期値を代入して、未初期化エラーを防ぐ
    t = 0.0f;
    normal = float3(0.0f, 0.0f, 0.0f);

    // カメラから球体の中心へのベクトル
    float3 oc = ray.origin - sphere.center;
    
    // 二次方程式 (at^2 + 2bt + c = 0) の係数を計算
    float a = dot(ray.direction, ray.direction);
    float b = dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    
    // 判別式 (D / 4 = b^2 - ac)
    float discriminant = b * b - a * c;
    
    // 判別式が0以上なら、光線が球体に当たっている！
    if (discriminant > 0.0f)
    {
        // 一番手前で当たった位置までの距離 (t) を計算
        t = (-b - sqrt(discriminant)) / a;
        if (t > 0.0f)
        {
            // 当たった場所の座標から、球体の「法線（表面の向き）」を計算
            float3 hitPoint = ray.origin + ray.direction * t;
            normal = normalize(hitPoint - sphere.center);
            return true;
        }
    }
    return false;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float width, height;
    g_outputTex.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;

    float2 uv = float2(((float) DTid.x / width) * 2.0f - 1.0f, (((float) DTid.y / height) * 2.0f - 1.0f) * -1.0f);
    uv.x *= (width / height);

    Ray ray;
    ray.origin = float3(0.0f, 0.0f, -5.0f);
    ray.direction = normalize(float3(uv, 1.0f));

    // 1. 2つの物体を定義
    Sphere targetSphere; // 赤い球体
    targetSphere.center = float3(0.0f, 0.5f, 0.0f); // 少し上に浮かせる
    targetSphere.radius = 1.0f;
    targetSphere.color = float3(0.9f, 0.2f, 0.2f);

    Sphere floorSphere; // 地面（めちゃくちゃデカい球体の一部分を床に見立てる）
    floorSphere.center = float3(0.0f, -101.0f, 0.0f); // はるか下に配置
    floorSphere.radius = 100.0f; // 半径100の巨大球
    floorSphere.color = float3(0.7f, 0.7f, 0.7f); // 白い床

    // 背景色
    float3 finalColor = float3(0.1f, 0.1f, 0.13f);
    float3 lightDir = normalize(float3(1.0f, 1.0f, -1.0f));

    // 2. まずは通常通りカメラからのレイの当たり判定
    float t;
    float3 normal;
    bool hit = false;
    Sphere hitSphere;

    // 赤い球体か、床のどちらに当たったか判定
    float t1, t2;
    float3 n1, n2;
    bool hit1 = HitSphere(ray, targetSphere, t1, n1);
    bool hit2 = HitSphere(ray, floorSphere, t2, n2);

    if (hit1 && (!hit2 || t1 < t2))
    {
        hit = true;
        t = t1;
        normal = n1;
        hitSphere = targetSphere;
    }
    else if (hit2)
    {
        hit = true;
        t = t2;
        normal = n2;
        hitSphere = floorSphere;
    }

    // 3. 物体に当たっていたら、影の計算（シャドウレイ）をする
    if (hit)
    {
        float3 hitPoint = ray.origin + ray.direction * t;

        // シャドウレイ（影の計算）
        Ray shadowRay;
        shadowRay.origin = hitPoint + normal * 0.001f;
        shadowRay.direction = lightDir;

        bool inShadow = false;
        float st;
        float3 sn;
        if (hitSphere.center.y < 0.0f)
        { // 今見ているのが「床」なら
            if (HitSphere(shadowRay, targetSphere, st, sn))
            {
                inShadow = true; // 赤い球の影の中
            }
        }

        // 基本の光の計算
        float diffuse = max(dot(normal, lightDir), 0.0f);
        float ambient = 0.15f; // 影の中でも真っ暗にならないための環境光

        if (inShadow)
        {
            diffuse = 0.0f; // 影の中なら直接光（太陽）をゼロにする
        }

        // 物体そのものの色（ベースカラー）
        float3 baseColor = hitSphere.color * (diffuse + ambient);

        // リフレクションレイ（反射の計算）
        Ray reflectRay;
        reflectRay.origin = hitPoint + normal * 0.001f;
        reflectRay.direction = reflect(ray.direction, normal);

        float3 reflectedColor = float3(0.1f, 0.1f, 0.13f); // デフォルトは背景の闇の色
        
        float rt;
        float3 rn;
        // 反射した光線が何かに当たるかチェック
        if (HitSphere(reflectRay, targetSphere, rt, rn))
        {
            reflectedColor = targetSphere.color * 0.5f; // 赤い球が映り込む
        }
        else if (HitSphere(reflectRay, floorSphere, rt, rn))
        {
            reflectedColor = floorSphere.color * 0.5f; // 床が映り込む
        }

        // 影（ベース色）と反射を同居させる
        // 物体の本来の色に、反射した色を乗せる（10%くらい鏡の要素を混ぜるイメージ）
        float reflectionStrength = 0.1f;
        finalColor = lerp(baseColor, reflectedColor, reflectionStrength);
    }

    g_outputTex[DTid.xy] = float4(finalColor, 1.0f);
}