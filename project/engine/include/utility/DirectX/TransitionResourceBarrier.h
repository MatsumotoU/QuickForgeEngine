#pragma once
#include <d3d12.h>

namespace QFE {
	class TransitionResourceBarrier {
	public:
		static void Transition(
			ID3D12GraphicsCommandList* list, ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	};
}