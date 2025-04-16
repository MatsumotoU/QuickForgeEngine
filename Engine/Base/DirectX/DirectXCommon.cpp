#include "DirectXCommon.h"
#include "../MyString.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

DirectXCommon* DirectXCommon::GetInstatnce() {
	static DirectXCommon instance;
	return &instance;
}

void DirectXCommon::Initialize() {
	Log(ConvertString(std::format(L"DirectXCommon:StartInitialize!\n")));

	// オブジェクト解放用ポインタの初期化
	resources_.clear();

	// windowsの管理クラスのインスタンス取得
	winApp_ = WinApp::GetInstance();

	// デバッグログ
	debugLog_ = MyDebugLog::GetInstatnce();
	debugLog_->Log("DirectXCommon:StartInitialize");

	// DirectXの初期化
	Log(ConvertString(std::format(L"DirectXCommon:StartDirectInitialize!\n")));
	debugLog_->Log("DirectXCommon:StartDirectInitialize");
	CreateDxgiFactory();
	FindAdapter();
	CreateDevice();
	Log(ConvertString(std::format(L"DirectXCommon:EndDirectInitialize!\n")));
	debugLog_->Log("DirectXCommon:EndDirectInitialize");

	// 描画前の設定
	CreateCommandAllocator();
	CreateCommandList();
	CreateCommandQueue();
	CreateSwapChain();
	InitializeSwapChainResource();
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	CreateRTV();
	CreateFence();
	CreateFenceEvent();
}

void DirectXCommon::ReleaseDirectXObject() {
	// ID3D12Resourceで生成されたリソース解放
	for (int32_t index = 0; index < static_cast<int32_t>(resources_.size()); ++index) {
		resources_[index]->Release();
	}

	CloseHandle(fenceEvent_);
	// コマンドリストとスワップチェインの解放
	commandQueue_->Release();
	commandAllocator_->Release();
	commandList_->Release();
	swapChain_->Release();

	// DirectXの元締め
	device_->Release();
	useAdapter_->Release();
	device_->Release();
}

void DirectXCommon::PreDraw() {
	// * コマンドを積み込む * //
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResource_[backBufferIndex];
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// バリア張る
	commandList_->ResourceBarrier(1, &barrier);

	// 画面を青っぽい色で初期化
	InitializeBackGround(0.1f, 0.25f, 0.5f, 1.0f);
}

void DirectXCommon::PostDraw() {
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResource_[backBufferIndex];
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// バリア張る
	commandList_->ResourceBarrier(1, &barrier);

	// コマンドリストの内容を確定
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// * コマンドをキックする * //
	ID3D12CommandList* commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists);
	// GPUとOSに画面の交換を行うように通知
	swapChain_->Present(1, 0);

	// FenceValueの値を更新
	fenceValue_++;
	// GPUがここまでたどり着いたときに、Fenceのあたいを指定したあたいに代入するようにSignalを送る
	commandQueue_->Signal(fence_, fenceValue_);

	// Fenceの値が指定したSignal値にたどり着いてるか確認
	// GetCompletedValueの初期値はFencesakuseijini 渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_, nullptr);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeBackGround(float red, float green, float blue, float alpha) {

	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();
	// 描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { red,green,blue,alpha };
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
}

ID3D12Resource* DirectXCommon::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// * VertexResourceを生成する * //
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
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	// 解放用のポインタを保持
	

	return vertexResource;
}

ID3D12Device* DirectXCommon::GetDevice() {
	return device_;
}

ID3D12GraphicsCommandList* DirectXCommon::GetCommandList() {
	return commandList_;
}

DXGI_SWAP_CHAIN_DESC1* DirectXCommon::GetSwapChainDesc() {
	return &swapChainDesc_;
}

IDXGISwapChain4* DirectXCommon::GetSwapChain() {
	return swapChain_;
}

D3D12_RENDER_TARGET_VIEW_DESC* DirectXCommon::GetRtvDesc() {
	return &rtvDesc_;
}

D3D12_CPU_DESCRIPTOR_HANDLE* DirectXCommon::GetRtvHandles() {
	return &rtvHandles_[swapChain_->GetCurrentBackBufferIndex()];
}

void DirectXCommon::CreateDxgiFactory() {
	assert(!dxgiFactory_);
	// DXGIファクトリーの生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::FindAdapter() {
	// digiFactoryが生成できてないなら起動させない
	assert(!useAdapter_);
	assert(dxgiFactory_);

	// * アダプタの選定 * //
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			debugLog_->Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
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
		HRESULT hr = D3D12CreateDevice(useAdapter_, featureLevels[i], IID_PPV_ARGS(&device_));
		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力してループ脱出
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			debugLog_->Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイス生成が上手くいかなかったので起動できない
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!\n");
	debugLog_->Log("Complete create D3D12Device");

	// エラー落ち処理
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		// ヤバエラー落ち
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー落ち
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

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
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:CreateCommandQueue!\n")));
	debugLog_->Log("DirectXCommon:CreateCommandQueue");
}

void DirectXCommon::CreateCommandAllocator() {
	assert(!commandAllocator_);

	// コマンドアロケータを生成する
	HRESULT hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成例外
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:CreateCommandAllocator!\n")));
	debugLog_->Log("DirectXCommon:CreateCommandAllocator");
}

void DirectXCommon::CreateCommandList() {
	// コマンドリスト生成
	HRESULT hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr,
		IID_PPV_ARGS(&commandList_));
	// コマンドリスト生成例外
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:CreateCommandList!\n")));
	debugLog_->Log("DirectXCommon:CreateCommandList");
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
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, winApp_->GetHWND(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:CreateSwapChain!\n")));
	debugLog_->Log("DirectXCommon:CreateSwapChain");
}

ID3D12DescriptorHeap* DirectXCommon::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:CreateDescriptorHeap!\n")));
	return descriptorHeap;
}

void DirectXCommon::InitializeSwapChainResource() {
	swapChainResource_[0] = nullptr;
	swapChainResource_[1] = nullptr;
	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResource_[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResource_[1]));
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"DirectXCommon:InitializeSwapChainResource!\n")));
}

void DirectXCommon::CreateRTV() {
	// * RTVを作る * //
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// まず1つ目を作る。
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResource_[0], &rtvDesc_, rtvHandles_[0]);
	// 二つ目
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResource_[1], &rtvDesc_, rtvHandles_[1]);
	Log(ConvertString(std::format(L"DirectXCommon:CreateRTV!\n")));
}

void DirectXCommon::CreateFence() {
	// * FenceとEventを生成する * //
	// 初期値０でFenceを作る
	fence_ = nullptr;
	fenceValue_ = 0;
	HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateFenceEvent() {
	// FenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}