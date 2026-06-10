#pragma once
#include <d3d12.h>
#include <unordered_map>

namespace QFE::GRAPHIC::INTERNAL {
	enum class BlendMode {
		// ブレンドなし
		kBlendModeNone,
		// 通常
		kBlendModeNormal,
		// 加算
		kBlendModeAdd,
		// 減算
		kBlendModeSubtract,
		// 乗算
		kBlendModeMultiply,
		// スクリーン
		kBlendModeScreen,
		// 利用禁止
		kCountOfBlendMode,
	};

	/// @brief ブレンドモードを管理するクラス
	class BlendStates final {
	public:
		/// @brief ブレンドステートの設定を生成します
		void Initialize();
		/// @brief 指定したBlendModeに対応するブレンドステートの設定を取得します
		const D3D12_BLEND_DESC& GetBlendDesc(BlendMode mode) const;
	private:
		std::unordered_map<BlendMode, D3D12_BLEND_DESC> blendDescs_;
	};
}