#pragma once
#include <wrl.h>
#include <d3d12.h>

class SrvDescriptorHeap final {
public:// 一度は呼び出さないとバグるやつ
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible);

public:// メンバ関数

public:// デバッグ用機能
#ifdef _DEBUG
	/// <summary>
	/// SRVディスクリプターヒープの状況を表示します
	/// </summary>
	void DrawDebugWindow();
#endif // _DEBUG

public:
	ID3D12DescriptorHeap* GetSrvDescriptorHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
};