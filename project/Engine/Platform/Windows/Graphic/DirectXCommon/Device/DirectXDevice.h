#pragma once
#include <wrl.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

class DirectXDevice final {
public:
	DirectXDevice();
	~DirectXDevice();
	void Initialize();
	void Shutdown();

	[[nodiscard]] IDXGIFactory7* GetDxgiFactory() const;
	[[nodiscard]] ID3D12Device* GetDevice() const;
	[[nodiscard]] IDXGIAdapter4* GetUseAdapter() const;
#ifdef _DEBUG
	void SetDisableError(bool disable);
	void SetDisableWarning(bool disable);
#endif // _DEBUG

private:
	void CreateDxgiFactory();
	void FindAdapter();
	void CreateDevice();

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
#ifdef _DEBUG
	bool disableError_;
	bool disableWarning_;
#endif // _DEBUG
};