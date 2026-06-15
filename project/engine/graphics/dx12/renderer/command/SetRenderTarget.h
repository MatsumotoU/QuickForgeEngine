#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"

#include <functional>

namespace QFE::GRAPHIC {
	/// @brief 描画ターゲットを設定するコマンド
	class SetRenderTarget final : public INTERNAL::IRenderCommand {
	public:
		/// @brief 描画ターゲットを設定するコマンドを生成します。
		explicit SetRenderTarget(
			RenderTargetHandle renderTarget, DepthStencilHandle depthStencil,
			std::function<void(RenderTargetHandle, DepthStencilHandle)>setRenderTargetFunc);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute(ID3D12GraphicsCommandList* commandList) override;

	private:
		RenderTargetHandle renderTargetHandle_;
		DepthStencilHandle depthStencilHandle_;
		std::function<void(RenderTargetHandle, DepthStencilHandle)> setRenderTargetFunc_;
	};
}
