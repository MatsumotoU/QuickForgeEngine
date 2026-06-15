#pragma once
#include <d3d12.h>

namespace QFE::GRAPHIC::INTERNAL {
	class IRenderCommand {
	public:
		virtual ~IRenderCommand() = default;
		/// @brief コマンドリストに描画コマンドを発行
		virtual void Execute(ID3D12GraphicsCommandList* commandList) = 0;
	};
}
