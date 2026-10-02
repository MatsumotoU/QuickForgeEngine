#pragma once
namespace QFE::GRAPHIC {
	/// @brief グラフィックエンジンの機能が有効かどうかを表す構造体
	struct FeatureState final {
		bool isRaytracingEnabled = false; ///< レイトレーシング機能が有効かどうか
		bool isDLSSupported = false;      ///< DLSS機能が有効かどうか
	};
}