#pragma once
#include "../Windows/WinApp.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

class DirectXCommon {
public:
	// シングルトン
	static DirectXCommon* GetInstatnce();

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	IDXGIFactory7* CreateDXGIFactory();
	IDXGIAdapter4* FindAdapter();
	ID3D12Device* CreateDivice();


private:// メンバ変数
	// windowsアプリケーション管理
	WinApp* winApp_;

	IDXGIFactory7* dxgiFactory_;
	ID3D12Device* divice_;
	IDXGIAdapter4* useAdapter_;

private:
	// シングルトン用
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

};