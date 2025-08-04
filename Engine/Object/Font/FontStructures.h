#pragma once
#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h> // DirectXMath for matrix/vector operations

#include "Math/Vector/Vector2.h"

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

struct FontDataToShader {
	Vector2 AtlasSize; // アトラスのピクセルサイズ (width, height)
	float DistanceRange; // JSONから読み込んだ distanceRange (pixelRange)
	float padding;
};

struct FontUVData {
	Vector2 leftTop;    // 左上のUV座標
	Vector2 rightTop;   // 右上のUV座標
	Vector2 leftBottom; // 左下のUV座標
	Vector2 rightBottom; // 右下のUV座標
};