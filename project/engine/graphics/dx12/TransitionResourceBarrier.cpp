#include "TransitionResourceBarrier.h"
using namespace QFE;
void TransitionResourceBarrier::Transition(
	ID3D12GraphicsCommandList* list, ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {

	// 繝舌Μ繧｢
	D3D12_RESOURCE_BARRIER barrier{};
	// 莉雁屓縺ｮ繝舌Μ繧｢縺ｯ繝医Λ繝ｳ繧ｸ繧ｷ繝ｧ繝ｳ
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// None縺ｫ縺吶ｋ
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// 繝舌Μ繧｢繧定ｲｼ繧句ｯｾ雎｡縺ｮ繝ｪ繧ｽ繝ｼ繧ｹ縲ら樟蝨ｨ縺ｮ繝舌ャ繧ｯ繝舌ャ繝輔ぃ縺ｫ蟇ｾ縺励※陦後≧
	barrier.Transition.pResource = pResource;
	// 驕ｷ遘ｻ蜑搾ｼ育樟蝨ｨ・峨・繝ｪ繧ｽ繝ｼ繧ｹ繧ｹ繝・・繝・
	barrier.Transition.StateBefore = before;
	// 驕ｷ遘ｻ蠕後・ResourceState
	barrier.Transition.StateAfter = after;
	// TODO: 隍・焚蝗樒┌鬧・↓繧ｳ繝槭Φ繝峨ｒ騾√ｉ縺ｪ縺・ｈ縺・↓縺吶ｋ
	// 繝舌Μ繧｢蠑ｵ繧・
	list->ResourceBarrier(1, &barrier);
}
