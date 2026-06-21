#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"

#include <functional>

namespace QFE::GRAPHIC {
	/// @brief 描画のプリミティブトポロジーを設定するコマンド
	class SetPrimitiveTopology final : public IRenderCommand {
	public:
		/// @brief 描画のプリミティブトポロジーを設定するコマンドを生成します。
		explicit SetPrimitiveTopology(
			D3D_PRIMITIVE_TOPOLOGY topology);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute(ID3D12GraphicsCommandList* commandList) override;

	private:
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology_;
	};
}
