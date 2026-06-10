#pragma once
#include <d3d12.h>
#include <unordered_map>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief RasterizerStateの設定の種類を表す列挙型
	enum class RasterizerType {
		Default,
		Wireframe,
		CullNone,
	};

	/// @brief PSOのRasterizerStateの設定を生成するためのクラス
	class RasterizerTemplate final {
	public:
		/// @brief RasterizerStateの設定を生成します
		void Initialize();

		/// @brief 指定したRasterizerTypeに対応するRasterizerStateの設定を取得します
		const D3D12_RASTERIZER_DESC& GetRasterizerDesc(RasterizerType type) const;
	
	private:
		std::unordered_map<RasterizerType, D3D12_RASTERIZER_DESC> rasterizerDescs_;

	};
}