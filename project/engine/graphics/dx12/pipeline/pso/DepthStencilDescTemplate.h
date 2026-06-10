#pragma once
#include <d3d12.h>
#include <unordered_map>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief 深度値比較方法
	enum class DepthStencilDescType {
		Default,// 標準、比較あり、書き込みあり
		Translucent,// 半透明につかう.比較あり、書きこみなし
		None// UIとかに使う.比較も書き込みもしない
	};

	/// @brief 深度ステンシルステートのテンプレートクラス
	class DepthStencilDescTemplate {
	public:
		/// @brief テンプレートの初期化
		void Initialize();
		/// @brief 指定したタイプの深度ステンシルステートを取得
		const D3D12_DEPTH_STENCIL_DESC& GetDesc(DepthStencilDescType type) const;

	private:
		std::unordered_map<DepthStencilDescType, D3D12_DEPTH_STENCIL_DESC > descs;
	};
}
