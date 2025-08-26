#include "TransitionResourceBarrier.h"

void TransitionResourceBarrier::Transition(
	ID3D12GraphicsCommandList* list, ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {

	// バリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = pResource;
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = before;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = after;
	// TODO: 複数回無駄にコマンドを送らないようにする
	// バリア張る
	list->ResourceBarrier(1, &barrier);
}
