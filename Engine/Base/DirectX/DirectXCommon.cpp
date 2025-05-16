#include "DirectXCommon.h"
#include "../MyString.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include "../Windows/WinApp.h"

void DirectXCommon::Initialize(WinApp* winApp) {
	// windowsの管理クラス取得
	winApp_ = winApp;

	// デバッグログ
	debugLog_ = MyDebugLog::GetInstatnce();
	debugLog_->Log("-----DirectXCommon:StartInitialize-----\n");

#ifdef _DEBUG
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController_.GetAddressOf())))) {
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

	// DirectXの初期化
	debugLog_->Log("DirectXCommon:StartDirectInitialize");
	CreateDxgiFactory();
	FindAdapter();
	CreateDevice();
	debugLog_->Log("DirectXCommon:EndDirectInitialize");

	// descriptorHeapSizeの代入
	descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// 描画前の設定
	CreateCommandAllocator();
	CreateCommandList();
	CreateCommandQueue();
	CreateSwapChain();
	InitializeSwapChainResource();
	InitializeOffScreenResource();
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);// こいつがRTV本体
	CreateRTV();// 正確にいうとRTVにリソースを登録している
	CreateOffScreenRTV();
	CreateFence();
	CreateFenceEvent();
}

void DirectXCommon::ReleaseDirectXObject() {
	CloseHandle(fenceEvent_);
}

void DirectXCommon::PreDraw() {
	// * コマンドを積み込む * //
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_.Get()->GetCurrentBackBufferIndex();

	// バックバッファのバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResource_[backBufferIndex].Get();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// バリア張る
	commandList_.Get()->ResourceBarrier(1, &barrier);

	// 画面を青っぽい色で初期化
	InitializeBackGround(0.1f, 0.25f, 0.5f, 1.0f);
}

void DirectXCommon::PostDraw() {
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_.Get()->GetCurrentBackBufferIndex();

	// バリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResource_[backBufferIndex].Get();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// バリア張る
	commandList_.Get()->ResourceBarrier(1, &barrier);

	// コマンドリストの内容を確定
	HRESULT hr = commandList_.Get()->Close();
	assert(SUCCEEDED(hr));

	// * コマンドをキックする * //
	ID3D12CommandList* commandLists[] = { commandList_.Get()};
	commandQueue_.Get()->ExecuteCommandLists(1, commandLists);
	// GPUとOSに画面の交換を行うように通知
	swapChain_.Get()->Present(1, 0);

	// FenceValueの値を更新
	fenceValue_++;
	// GPUがここまでたどり着いたときに、Fenceのあたいを指定したあたいに代入するようにSignalを送る
	commandQueue_.Get()->Signal(fence_.Get(), fenceValue_);

	// Fenceの値が指定したSignal値にたどり着いてるか確認
	// GetCompletedValueの初期値はFencesakuseijini 渡した初期値
	if (fence_.Get()->GetCompletedValue() < fenceValue_) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_.Get()->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_.Get()->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeBackGround(float red, float green, float blue, float alpha) {
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_.Get()->GetCurrentBackBufferIndex();
	// 描画先のRTVを設定する
	commandList_.Get()->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { red,green,blue,alpha };
	commandList_.Get()->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
	float offScreenClearColor[4]{};
	offScreenClearColor[0] = 0.1f;
	offScreenClearColor[1] = 0.25f;
	offScreenClearColor[2] = 0.5f;
	offScreenClearColor[3] = 1.0f;
	commandList_.Get()->ClearRenderTargetView(offScreenRtvHandle_, offScreenClearColor, 0, nullptr);
}

ID3D12Device* DirectXCommon::GetDevice() {
	return device_.Get();
}

ID3D12GraphicsCommandList* DirectXCommon::GetCommandList() {
	return commandList_.Get();
}

DXGI_SWAP_CHAIN_DESC1* DirectXCommon::GetSwapChainDesc() {
	return &swapChainDesc_;
}

IDXGISwapChain4* DirectXCommon::GetSwapChain() {
	return swapChain_.Get();
}

D3D12_RENDER_TARGET_VIEW_DESC* DirectXCommon::GetRtvDesc() {
	return &rtvDesc_;
}

D3D12_CPU_DESCRIPTOR_HANDLE* DirectXCommon::GetRtvHandles() {
	return &rtvHandles_[swapChain_.Get()->GetCurrentBackBufferIndex()];
}

D3D12_CPU_DESCRIPTOR_HANDLE* DirectXCommon::GetOffscreenRtvHandles() {
	return &offScreenRtvHandle_;
}

ID3D12Resource* DirectXCommon::GetOffscreenResource() {
	return offScreenResource_.Get();
}

uint32_t DirectXCommon::GetDescriptorSizeSRV() {
	return descriptorSizeSRV_;
}

uint32_t DirectXCommon::GetDescriptorSizeRTV() {
	return descriptorSizeRTV_;
}

uint32_t DirectXCommon::GetDescriptorSizeDSV() {
	return descriptorSizeDSV_;
}

#ifdef _DEBUG
void DirectXCommon::SetCommandLine(LPSTR* lpCmdLine) {
	lpCmdLine_ = lpCmdLine;
}
#endif // _DEBUG

void DirectXCommon::CreateDxgiFactory() {
	assert(!dxgiFactory_);
	// DXGIファクトリーの生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	hr;
	assert(SUCCEEDED(hr));
}

void DirectXCommon::FindAdapter() {
	// digiFactoryが生成できてないなら起動させない
	assert(!useAdapter_);
	assert(dxgiFactory_);

	// * アダプタの選定 * //
	for (UINT i = 0; dxgiFactory_.Get()->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter_.Get()->GetDesc3(&adapterDesc);
		hr;
		assert(SUCCEEDED(hr));

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。
#ifdef _DEBUG
			debugLog_->Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
#endif // _DEBUG
			break;
		}
		useAdapter_ = nullptr;
	}

	// 適切なアダプタを見つけられなかったので起動できない
	assert(useAdapter_ != nullptr);
}

void DirectXCommon::CreateDevice() {
	// digiFactoryが生成できてないなら起動させない
	assert(!device_);
	assert(useAdapter_);

	// * D3D12Deviceの生成 * //
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	// 高い順に生成できるか試す
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを作成
		HRESULT hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力してループ脱出
#ifdef _DEBUG
			debugLog_->Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
#endif // _DEBUG
			break;
		}
	}
	// デバイス生成が上手くいかなかったので起動できない
	assert(device_ != nullptr);
	
#ifdef _DEBUG
	debugLog_->Log("Complete create D3D12Device");
#endif // _DEBUG

	// エラー落ち処理
#ifdef _DEBUG
	debugLog_->Log("EnebleBreakOnSeverity");
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		// ヤバエラー落ち
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		debugLog_->Log("EnebleBreakOnSeverity_CORRUPTION");
		// エラー落ち
		if (lpCmdLine_ == nullptr || std::strcmp(*lpCmdLine_, "/DISABLE_D3D12_DEBUG_ERROR") != 0) {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			debugLog_->Log("EnebleBreakOnSeverity_ERROR");
		} else {
			debugLog_->Log("!!! DisableBreakOnSeverity_ERROR !!!");
		}
		// 警告
		if (lpCmdLine_ == nullptr || std::strcmp(*lpCmdLine_, "/DISABLE_D3D12_DEBUG_WARNING")  != 0) {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			debugLog_->Log("EnebleBreakOnSeverity_WARNING");
		} else {
			debugLog_->Log("!!! DisableBreakOnSeverity_WARNING !!!");
		}

		// エラー抑制
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制レベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定メッセージを抑制
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue->Release();
	}
#endif // _DEBUG
}

void DirectXCommon::CreateCommandQueue() {
	assert(!commandQueue_);

	// * コマンドクエリの生成 * //
	commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	hr;
	assert(SUCCEEDED(hr));
	
#ifdef _DEBUG
	debugLog_->Log("DirectXCommon:CreateCommandQueue");
#endif // _DEBUG
}

void DirectXCommon::CreateCommandAllocator() {
	assert(!commandAllocator_);

	// コマンドアロケータを生成する
	HRESULT hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.GetAddressOf()));
	// コマンドアロケータの生成例外
	hr;
	assert(SUCCEEDED(hr));
	
#ifdef _DEBUG
	debugLog_->Log("DirectXCommon:CreateCommandAllocator");
#endif // _DEBUG
}

void DirectXCommon::CreateCommandList() {
	// コマンドリスト生成
	HRESULT hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(commandList_.GetAddressOf()));
	// コマンドリスト生成例外
	hr;
	assert(SUCCEEDED(hr));
	
#ifdef _DEBUG
	debugLog_->Log("DirectXCommon:CreateCommandList");
#endif // _DEBUG
}

void DirectXCommon::CreateSwapChain() {
	// * SwapChainを生成する * //
	swapChainDesc_ = {};
	swapChainDesc_.Width = winApp_->kWindowWidth;
	swapChainDesc_.Height = winApp_->kWindowHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成
	HRESULT hr = dxgiFactory_.Get()->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHWND(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
	
#ifdef _DEBUG
	debugLog_->Log("DirectXCommon:CreateSwapChain");
#endif // _DEBUG
}

void DirectXCommon::InitializeSwapChainResource() {
	swapChainResource_[0] = nullptr;
	swapChainResource_[1] = nullptr;
	HRESULT hr = swapChain_.Get()->GetBuffer(0, IID_PPV_ARGS(&swapChainResource_[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain_.Get()->GetBuffer(1, IID_PPV_ARGS(&swapChainResource_[1]));
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	debugLog_->Log("DirectXCommon:InitializeSwapChainResource");
#endif // _DEBUG
}

void DirectXCommon::InitializeOffScreenResource() {
	offScreenDesc_ = {};
	offScreenDesc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	offScreenDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	offScreenDesc_.Width = winApp_->kWindowWidth;
	offScreenDesc_.Height = winApp_->kWindowHeight;
	offScreenDesc_.DepthOrArraySize = 1; // テクスチャなので通常は1
	offScreenDesc_.MipLevels = 1;       // 通常は1
	offScreenDesc_.SampleDesc.Count = 1;
	offScreenDesc_.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使用

	offscreenClearValue_ = {};
	offscreenClearValue_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	offscreenClearValue_.Color[0] = 0.1f;
	offscreenClearValue_.Color[1] = 0.25f;
	offscreenClearValue_.Color[2] = 0.5f;
	offscreenClearValue_.Color[3] = 1.0f;

	D3D12_HEAP_PROPERTIES defaultHeapProperties{};
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HRESULT hr = device_->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&offScreenDesc_,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態
		&offscreenClearValue_,
		IID_PPV_ARGS(&offScreenResource_));
	hr;
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateRTV() {
	// * RTVを作る * //
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// まず1つ目を作る。
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResource_[0].Get(), &rtvDesc_, rtvHandles_[0]);
	// 二つ目
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResource_[1].Get(), &rtvDesc_, rtvHandles_[1]);
}

void DirectXCommon::CreateOffScreenRTV() {
	// スワップチェインの後にオフスク用RTV作成
	offScreenRtvHandle_.ptr = rtvHandles_[1].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(offScreenResource_.Get(), &rtvDesc_, offScreenRtvHandle_);
}

void DirectXCommon::CreateFence() {
	// * FenceとEventを生成する * //
	// 初期値０でFenceを作る
	fence_ = nullptr;
	fenceValue_ = 0;
	HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	hr;
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateFenceEvent() {
	// FenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDecriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDecriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// * Resourceを生成する * //
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(vertexResource.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	MyDebugLog::GetInstatnce()->Log(ConvertString(std::format(L"CreateBufferResource Bytes = {}", sizeInBytes)));
#endif // _DEBUG
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	MyDebugLog::GetInstatnce()->Log("DirectXCommon:CreateDescriptorHeap");
#endif // _DEBUG
	return descriptorHeap;
}
