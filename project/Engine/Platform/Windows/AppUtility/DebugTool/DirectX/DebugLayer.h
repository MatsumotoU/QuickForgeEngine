#pragma once
#ifdef _DEBUG
#include <d3d12.h>
#include <wrl.h>

class DebugLayer final {
public:
	DebugLayer();
	~DebugLayer() = default;
private:
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;
};
#endif // _DEBUG