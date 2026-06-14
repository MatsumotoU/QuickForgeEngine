#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"

#include <functional>
#include <vector>
#include <d3d12.h>

namespace QFE::GRAPHIC {
	/// @brief 描画のプリミティブトポロジーを設定するコマンド
	class DrawOneInstanced final : public INTERNAL::IRenderCommand {
	public:
		/// @brief 描画のプリミティブトポロジーを設定するコマンドを生成します。
		explicit DrawOneInstanced(std::);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute() override;

	private:
	};
}
