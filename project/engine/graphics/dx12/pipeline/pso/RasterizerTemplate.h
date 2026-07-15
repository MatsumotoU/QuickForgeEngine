#pragma once
#include <d3d12.h>
#include <unordered_map>
#include "PipelineDescTypes.h"

namespace QFE::GRAPHIC {
	/// @brief PSOのRasterizerStateの設定を生成するためのクラス
	class RasterizerTemplate final {
	public:
		/// @brief RasterizerStateの設定を生成します
		void Initialize();

		/// @brief 指定したRasterizerTypeに対応するRasterizerStateの設定を取得します
		const D3D12_RASTERIZER_DESC& GetRasterizerDesc(RasterizerType type) const;
		/// @brief RasterizerTypeとRasterizerStateの設定のマップを取得します
		std::unordered_map<RasterizerType, D3D12_RASTERIZER_DESC> GetRasterizerDescMap() const;
		/// @brief RasterizerTypeの数を取得します
		uint32_t GetRasterizerTypeCount() const;
	
	private:
		std::unordered_map<RasterizerType, D3D12_RASTERIZER_DESC> rasterizerDescs_;

	};
}