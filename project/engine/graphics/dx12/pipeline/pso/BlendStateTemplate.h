#pragma once
#include <d3d12.h>
#include <unordered_map>
#include "PipelineDescTypes.h"

namespace QFE::GRAPHIC {
	/// @brief ブレンドモードを管理するクラス
	class BlendStateTemplate final {
	public:
		void Initialize();
		/// @brief 指定したBlendModeに対応するブレンドステートの設定を取得します
		D3D12_BLEND_DESC GetBlendDesc(BlendMode mode) const;
		/// @brief ブレンドモードとブレンドステートの設定のマップを取得します
		std::unordered_map<BlendMode, D3D12_BLEND_DESC> GetBlendDescMap() const;
		/// @brief BlendModeの数を取得します
		uint32_t GetBlendModeCount() const;
	private:
		std::unordered_map<BlendMode, D3D12_BLEND_DESC> blendDescMap_;
	};
}