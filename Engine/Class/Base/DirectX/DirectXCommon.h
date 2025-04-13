#pragma once
#include "../Windows/WinApp.h"

#include <vector>

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
	/// <summary>
	/// これまで生成してきたオブジェクトを解放する
	/// </summary>
	void ReleaseDirectXObject();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// リソースを作成
	/// </summary>
	/// <param name="device">device</param>
	/// <param name="sizeInBytes">サイズ</param>
	/// <returns>リソース</returns>
	static ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	/// <summary>
	/// ディスクリプタヒープを作成する
	/// </summary>
	static ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

public:
	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetCommandList();

private: // メンバ変数
	// windowsアプリケーション管理
	WinApp* winApp_;

	// DirectXの管理
	IDXGIFactory7* dxgiFactory_;
	ID3D12Device* device_;
	IDXGIAdapter4* useAdapter_;

	ID3D12CommandQueue* commandQueue_;
	ID3D12CommandAllocator* commandAllocator_;
	ID3D12GraphicsCommandList* commandList_;
	IDXGISwapChain4* swapChain_;
	ID3D12Resource* swapChainResource_[2];
	ID3D12DescriptorHeap* rtvDescriptorHeap_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	ID3D12Fence* fence_;
	HANDLE fenceEvent_;
	uint64_t fenceValue_;

	// リソース解放用ポインタ
	std::vector<ID3D12Resource*> resources_;

private: // 外部から触れてほしくない関数たち
	/// <summary>
	/// DXGIFactoryを生成します
	/// </summary>
	void CreateDxgiFactory();
	/// <summary>
	/// Adapterを選定します
	/// </summary>
	void FindAdapter();
	/// <summary>
	/// Diviceを生成する
	/// </summary>
	void CreateDevice();

	/// <summary>
	/// コマンドクエリを作成します
	/// </summary>
	void CreateCommandQueue();
	/// <summary>
	/// コマンドアロケータを作成する
	/// </summary>
	void CreateCommandAllocator();
	/// <summary>
	/// コマンドリストを作成する
	/// </summary>
	void CreateCommandList();
	/// <summary>
	/// スワップチェインを作成する
	/// </summary>
	void CreateSwapChain();
	/// <summary>
	/// スワップチェインからリソースを引っ張ってくる
	/// </summary>
	void InitializeSwapChainResource();
	/// <summary>
	/// RTVを作る
	/// </summary>
	void CreateRTV();
	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();
	/// <summary>
	/// フェンスイベントを生成
	/// </summary>
	void CreateFenceEvent();

	/// <summary>
	/// 背景の色を指定の色で初期化します
	/// </summary>
	/// <param name="red">赤(0.0~1.0)</param>
	/// <param name="green">緑(0.0~1.0)</param>
	/// <param name="blue">青(0.0~1.0)</param>
	/// <param name="alpha">透明度(0.0~1.0)</param>
	void InitializeBackGround(float red, float green, float blue, float alpha);

private: // シングルトン用
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

};