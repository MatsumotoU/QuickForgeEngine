#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectCommandList final {
public:
	DirectCommandList() = default;
	~DirectCommandList() = default;

	void Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
	void Reset();
	ID3D12GraphicsCommandList* GetCommandList() const;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
};