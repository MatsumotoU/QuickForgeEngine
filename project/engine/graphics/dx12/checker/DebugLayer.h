#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace QFE::GRAPHIC {
	class DebugLayer final {
	public:
		DebugLayer();
		~DebugLayer() = default;
	private:
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;
	};
}  // namespace QFE