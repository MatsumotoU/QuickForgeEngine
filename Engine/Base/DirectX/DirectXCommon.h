#pragma once
#include <wrl.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "../MyDebugLog.h"

class WinApp;

class DirectXCommon {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);
	/// <summary>
	/// 画面描画のリソース初期化(初期化の後に置くこと)
	/// </summary>
	/// <param name="rtvDescriptorHeap"></param>
	void InitializeScreenResources(ID3D12DescriptorHeap* rtvDescriptorHeap);

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

public:
	ID3D12Device*  GetDevice();
	ID3D12GraphicsCommandList* GetCommandList();
	DXGI_SWAP_CHAIN_DESC1* GetSwapChainDesc();
	IDXGISwapChain4* GetSwapChain();
	D3D12_RENDER_TARGET_VIEW_DESC* GetRtvDesc();
	D3D12_CPU_DESCRIPTOR_HANDLE* GetRtvHandles();
	D3D12_CPU_DESCRIPTOR_HANDLE* GetOffscreenRtvHandles();
	ID3D12Resource* GetOffscreenResource();

	uint32_t GetDescriptorSizeSRV();
	uint32_t GetDescriptorSizeRTV();
	uint32_t GetDescriptorSizeDSV();

#ifdef _DEBUG
	/// <summary>
	/// 初期化より前に置くこと
	/// </summary>
	/// <param name="lpCmdLine"></param>
	void SetCommandLine(LPSTR* lpCmdLine);
#endif // _DEBUG

private: // メンバ変数
	// windowsアプリケーション管理
	WinApp* winApp_;

	// DebugLog用
	MyDebugLog* debugLog_;

	// DirectXの管理
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource_[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> offScreenResource_;
	D3D12_RESOURCE_DESC offScreenDesc_;
	D3D12_CLEAR_VALUE offscreenClearValue_;
	D3D12_CPU_DESCRIPTOR_HANDLE offScreenRtvHandle_;
	ID3D12DescriptorHeap* rtvDescriptorHeap_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_;
	uint64_t fenceValue_;

	// 一回決めたら値が変わることがないやつら
	uint32_t descriptorSizeSRV_;
	uint32_t descriptorSizeRTV_;
	uint32_t descriptorSizeDSV_;

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;
	LPSTR* lpCmdLine_;
#endif // _DEBUG


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
	/// オフスクリーンのリソースを確保します
	/// </summary>
	void InitializeOffScreenResource();
	/// <summary>
	/// RTVを作る
	/// </summary>
	void CreateRTV();
	/// <summary>
	/// オフスクリーン用のRTVを作成します
	/// </summary>
	void CreateOffScreenRTV();
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
};

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDecriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDecriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

/// <summary>
/// リソースを作成
/// </summary>
/// <param name="device">device</param>
/// <param name="sizeInBytes">サイズ</param>
/// <returns>リソース</returns>
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
/// <summary>
/// ディスクリプタヒープを作成する
/// </summary>
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);