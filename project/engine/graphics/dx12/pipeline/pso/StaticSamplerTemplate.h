#pragma once
#include <d3d12.h>
#include <vector>

namespace QFE::GRAPHIC {
	/// @brief PSOのルートシグネチャで使用する静的サンプラーの定義クラス
	class StaticSamplerTemplate final {
	public:
		/// @brief 静的サンプラーの設定を生成します
		void Initialize();

		/// @brief 生成した静的サンプラーの設定を取得します
		const D3D12_STATIC_SAMPLER_DESC* GetSamplerDescs() const;
		/// @brief 生成した静的サンプラーの数を取得します
		UINT GetSamplerCount() const;

	private:
		/// @brief 静的サンプラーの設定が初期化されているかを確認します。初期化されていない場合はエラーを報告します。
		void CheckInitialized() const;

		bool isInitialized_ = false;
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescs_;
	};
}