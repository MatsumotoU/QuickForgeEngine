#pragma once
#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h> // DirectXMath for matrix/vector operations

struct AtlasBounds
{
    float left;
    float bottom;
    float right;
    float top;
};

struct PlaneBounds
{
    float left;
    float bottom;
    float right;
    float top;
};

struct GlyphInfo
{
    unsigned int unicode;
    float advance; // 次の文字に進むオフセット
    PlaneBounds planeBounds; // フォントのemスペース内でのグリフの境界
    AtlasBounds atlasBounds; // アトラス画像内でのピクセル単位の境界
};

struct FontMetrics
{
    float emSize;
    float lineHeight;
    float ascender;
    float descender;
    // 他のメトリクスもここに追加
};

struct FontAtlasData
{
    // Atlas情報
    std::string type;
    float distanceRange; // シェーダーで使うpxrange
    float width;         // アトラス画像の幅 (pixels)
    float height;        // アトラス画像の高さ (pixels)
    std::string yOrigin; // "bottom"

    // Metrics情報
    FontMetrics metrics;

    // グリフ情報 (unicode -> GlyphInfo のマップで高速検索)
    std::map<unsigned int, GlyphInfo> glyphs;
};