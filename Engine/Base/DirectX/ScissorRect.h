#pragma once
#include <d3d12.h>
#include <cstdint>

class WinApp;

class ScissorRect {
public:
	void Initialize(WinApp* winApp);

public:
	D3D12_RECT* GetScissorRect();
	void SetRight(const int32_t& right);
	void SetBottom(const int32_t& bottom);

private:
	D3D12_RECT scissorRect_;

};