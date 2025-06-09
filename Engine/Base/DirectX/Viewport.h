#pragma once
#include <d3d12.h>

class WinApp;

class ViewPort final {
public:
	void Inititalize(WinApp* winApp);

public:
	void SetWidth(const FLOAT& width);
	void SetHeight(const FLOAT& height);
	void SetMinDepth(const FLOAT& minDepth);
	void SetMaxDepth(const FLOAT& maxDepth);
	const D3D12_VIEWPORT* GetViewport();

private:

	D3D12_VIEWPORT viewport_;
};