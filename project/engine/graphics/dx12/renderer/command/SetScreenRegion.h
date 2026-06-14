#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"

#include <functional>

namespace QFE::GRAPHIC {
	/// @brief 描画ターゲットを設定するコマンド
	class SetScreenRegion final : public INTERNAL::IRenderCommand {
	public:
		/// @brief 描画ターゲットを設定するコマンドを生成します。
		explicit SetScreenRegion(
			ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
			std::function<void(ViewPortHandle)>setViewportFunc,
			std::function<void(ScissorRectHandle)>setScissorRectFunc);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute() override;

	private:
		ViewPortHandle viewportHandle_;
		ScissorRectHandle scissorRectHandle_;
		std::function<void(ViewPortHandle)> setViewportFunc_;
		std::function<void(ScissorRectHandle)> setScissorRectFunc_;
	};
}
