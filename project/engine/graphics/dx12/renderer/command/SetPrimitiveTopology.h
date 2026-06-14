#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"

#include <functional>
#include <d3d12.h>

namespace QFE::GRAPHIC {
	/// @brief 描画のプリミティブトポロジーを設定するコマンド
	class SetPrimitiveTopology final : public INTERNAL::IRenderCommand {
	public:
		/// @brief 描画のプリミティブトポロジーを設定するコマンドを生成します。
		explicit SetPrimitiveTopology(
			D3D_PRIMITIVE_TOPOLOGY topology,std::function<void(D3D_PRIMITIVE_TOPOLOGY)> setPrimitiveTopologyFunc);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute() override;

	private:
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology_;
		std::function<void(D3D_PRIMITIVE_TOPOLOGY)> setPrimitiveTopologyFunc_;
	};
}
