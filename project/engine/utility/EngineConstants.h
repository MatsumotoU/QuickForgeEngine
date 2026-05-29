#pragma once
#include <stdint.h>

/// @file EngineConstants.h
/// @brief エンジン全体で使用される定数の定義
namespace QFE::CONSTANTS {
	//* テクスチャマネージャの設定 * //
	namespace TEXTURE_MANAGER {
		// SafeVectorを使用しているため、メモリが一列予約されることを前提に、最大テクスチャ数と中間リソース数を定義
		constexpr uint32_t kMaxTextures = 500; // テクスチャマネージャが管理する最大テクスチャ数(1つで32byte)
		constexpr uint32_t kMaxIntermediateResources = 10; // テクスチャマネージャが管理する最大中間リソース数,1フレームの間に読み込めるテクスチャ数を想定して設定(1つ8byte)
	}
	// * GPUバッファープールの設定 * //
	namespace GPU_BUFFER_POOL {
		constexpr uint32_t kMaxConstantBuffers = 1000; // GPUバッファープールが管理する最大定数バッファ数
	}
}