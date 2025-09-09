#pragma once
#include <cstdint>
#include <vector>
#include <string>

class Vector3;

/// @brief マップチップの種類
enum class MapChipType {
	kBlank,	// 空白
	kDirt,	// 土
	kGrass,	// 草
	kStone,	// 石
};

/// @brief マップチップデータ
struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

/// @brief マップチップフィールド
class MapChipField {
public:
	// ブロックの大きさ
	static inline const float kBlockWidth = 1.0f;  // ブロックの幅
	static inline const float kBlockHeight = 1.0f; // ブロックの高さ

	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 8;		// 縦のブロック数
	static inline const uint32_t kNumBlockHorizontal = 8;	// 横のブロック数

	/// @brief XYインデックスのセット
	struct IndexSet {
		uint32_t xIndex; // 横のインデックス
		uint32_t yIndex; // 縦のインデックス
	};

	/// @brief CSVの読み込み
	/// @param filePath CSVファイルのパス
	void LoadMapChipCsv(const std::string &filePath);

	/// @brief マップチップの種類を取得
	/// @param xIndex 横のインデックス
	/// @param yIndex 縦のインデックス
	/// @return マップチップの種類
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;

private:
	MapChipData mapChipData_; // マップチップデータ

	/// @brief マップチップデータのリセット
	void ResetMapChipData();
};