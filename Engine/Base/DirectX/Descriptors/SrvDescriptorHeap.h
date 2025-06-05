#pragma once
#include <wrl.h>
#include <cstdint>
#include <d3d12.h>
#include "DescriptorHandles.h"

class SrvDescriptorHeap final {
public:// 一度は呼び出さないとバグるやつ
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible);

public:// メンバ関数
	/// <summary>
	/// 任意のヒープのGpuとCpuハンドルを取得する
	/// </summary>
	/// <param name="index">任意のヒープ</param>
	/// <returns></returns>
	DescriptorHandles AssignTextureHandles(const uint32_t& index);

	DescriptorHandles AssignOffscreenHandles(const uint32_t& index);

	DescriptorHandles AssignArrayHandles(const uint32_t& index);

	void AssignHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc,const D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescriptorHande);

public:// デバッグ用機能
#ifdef _DEBUG
	/// <summary>
	/// SRVディスクリプターヒープの状況を表示します
	/// </summary>
	void DrawDebugWindow();
#endif // _DEBUG

public:
	ID3D12DescriptorHeap* GetSrvDescriptorHeap();
	uint32_t GetTextureBeginIndex();
	uint32_t GetOffscreenBeginIndex();

private:
	ID3D12Device* device_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	uint32_t srvDescriptorHeapSize_;
	uint32_t textureBeginIndex_;
	uint32_t offscreenBeginIndex_;
	uint32_t arrayBeginIndex_;

	uint32_t descriptorSizeSRV_;
};