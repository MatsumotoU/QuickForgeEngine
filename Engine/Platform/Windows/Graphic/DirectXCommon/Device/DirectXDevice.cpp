#include "DirectXDevice.h"
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
#endif // DEBUG

DirectXDevice::DirectXDevice() {
#ifdef _DEBUG
	disableError_ = false;
	disableWarning_ = false;
#endif // DEBUG
}

DirectXDevice::~DirectXDevice() {
#ifdef _DEBUG
	DebugLog("-----DirectXDevice:Shutdown-----\n");
	DebugLog(std::format("Disable Error : {}\n", disableError_ ? "true" : "false"));
	DebugLog(std::format("Disable Warning : {}\n", disableWarning_ ? "true" : "false"));
#endif // _DEBUG
}

void DirectXDevice::Initialize() {
#ifdef _DEBUG
	DebugLog("-----DirectXDevice:Initialize-----\n");
	DebugLog(std::format("Disable Error : {}\n", disableError_ ? "true" : "false"));
	DebugLog(std::format("Disable Warning : {}\n", disableWarning_ ? "true" : "false"));
#endif // _DEBUG
	// DXGIファクトリーの生成
	CreateDxgiFactory();
	// アダプターの選定
	FindAdapter();
	// D3D12Deviceの生成
	CreateDevice();
#ifdef _DEBUG
	DebugLog("-----DirectXDevice:Initialize Complete-----\n");
#endif // _DEBUG
}

void DirectXDevice::Shutdown() {
}

IDXGIFactory7* DirectXDevice::GetDxgiFactory() const {
	return dxgiFactory_.Get();
}

ID3D12Device* DirectXDevice::GetDevice() const {
	return device_.Get();
}

IDXGIAdapter4* DirectXDevice::GetUseAdapter() const {
	return useAdapter_.Get();
}
#ifdef _DEBUG
void DirectXDevice::SetDisableError(bool disable) {
	disableError_ = disable;
	DebugLog(std::format("Disable Error : {}\n", disableError_ ? "true" : "false"));
}
void DirectXDevice::SetDisableWarning(bool disable) {
	disableWarning_ = disable;
	DebugLog(std::format("Disable Warning : {}\n", disableWarning_ ? "true" : "false"));
}
#endif // _DEBUG

void DirectXDevice::CreateDxgiFactory() {
	assert(!dxgiFactory_);
	// DXGIファクトリーの生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	hr;
	assert(SUCCEEDED(hr));
}

void DirectXDevice::FindAdapter() {
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
			DebugLog(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
#endif // _DEBUG
			break;
		}
		useAdapter_ = nullptr;
	}

	// 適切なアダプタを見つけられなかったので起動できない
	assert(useAdapter_ != nullptr);
}

void DirectXDevice::CreateDevice() {
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
			DebugLog(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
#endif // _DEBUG
			break;
		}
	}
	// デバイス生成が上手くいかなかったので起動できない
	assert(device_ != nullptr);

#ifdef _DEBUG
	DebugLog("Complete create D3D12Device");
#endif // _DEBUG

	// エラー落ち処理
#ifdef _DEBUG
	DebugLog("---EnebleBreakOnSeverity---");
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		// ヤバエラー落ち
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		DebugLog("EnebleBreakOnSeverity_CORRUPTION");
		// エラー落ち
		if (disableError_) {
			DebugLog("!!! DisableBreakOnSeverity_ERROR !!!");
		} else {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			DebugLog("EnebleBreakOnSeverity_ERROR");
		}
		// 警告
		if (disableWarning_) {
			DebugLog("!!! DisableBreakOnSeverity_WARNING !!!");
		} else {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			DebugLog("EnebleBreakOnSeverity_WARNING");
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
