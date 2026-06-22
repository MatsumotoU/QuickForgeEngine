#pragma once
#include <d3d12.h>
#include <unordered_map>
#include "PipelineDescTypes.h"

namespace QFE::GRAPHIC {
	/// @brief 深度ステンシルステートのテンプレートクラス
	class DepthStencilDescTemplate {
	public:
		/// @brief テンプレートの初期化
		void Initialize();
		/// @brief 指定したタイプの深度ステンシルステートを取得
		const D3D12_DEPTH_STENCIL_DESC& GetDesc(DepthStencilDescType type) const;
		/// @brief 深度ステンシルステートのマップを取得
		std::unordered_map<DepthStencilDescType, D3D12_DEPTH_STENCIL_DESC> GetDescMap() const;
		/// @brief 深度ステンシルステートの数を取得
		uint32_t GetDepthStencilDescTypeCount() const;

	private:
		std::unordered_map<DepthStencilDescType, D3D12_DEPTH_STENCIL_DESC > descs;
	};
}
