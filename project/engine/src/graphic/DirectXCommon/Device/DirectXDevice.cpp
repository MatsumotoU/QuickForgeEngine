#include "DirectXDevice.h"
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
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
	// DXGI繝輔ぃ繧ｯ繝医Μ繝ｼ縺ｮ逕滓・
	CreateDxgiFactory();
	// 繧｢繝繝励ち繝ｼ縺ｮ驕ｸ螳・
	FindAdapter();
	// D3D12Device縺ｮ逕滓・
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
	// DXGI繝輔ぃ繧ｯ繝医Μ繝ｼ縺ｮ逕滓・
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	hr;
	assert(SUCCEEDED(hr));
}

void DirectXDevice::FindAdapter() {
	// digiFactory縺檎函謌舌〒縺阪※縺ｪ縺・↑繧芽ｵｷ蜍輔＆縺帙↑縺・
	assert(!useAdapter_);
	assert(dxgiFactory_);

	// * 繧｢繝繝励ち縺ｮ驕ｸ螳・* //
	for (UINT i = 0; dxgiFactory_.Get()->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {

		// 繧｢繝繝励ち繝ｼ縺ｮ諠・ｱ繧貞叙蠕励☆繧・
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter_.Get()->GetDesc3(&adapterDesc);
		hr;
		assert(SUCCEEDED(hr));

		// 繧ｽ繝輔ヨ繧ｦ繧ｧ繧｢繧｢繝繝励ち縺ｧ縺ｪ縺代ｌ縺ｰ謗｡逕ｨ
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 謗｡逕ｨ縺励◆繧｢繝繝励ち縺ｮ諠・ｱ繧偵Ο繧ｰ縺ｫ蜃ｺ蜉帙・
#ifdef _DEBUG
			DebugLog(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
#endif // _DEBUG
			break;
		}
		useAdapter_ = nullptr;
	}

	// 驕ｩ蛻・↑繧｢繝繝励ち繧定ｦ九▽縺代ｉ繧後↑縺九▲縺溘・縺ｧ襍ｷ蜍輔〒縺阪↑縺・
	assert(useAdapter_ != nullptr);
}

void DirectXDevice::CreateDevice() {
	// digiFactory縺檎函謌舌〒縺阪※縺ｪ縺・↑繧芽ｵｷ蜍輔＆縺帙↑縺・
	assert(!device_);
	assert(useAdapter_);

	// * D3D12Device縺ｮ逕滓・ * //
	// 讖溯・繝ｬ繝吶Ν縺ｨ繝ｭ繧ｰ蜃ｺ蜉帷畑縺ｮ譁・ｭ怜・
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	// 鬮倥＞鬆・↓逕滓・縺ｧ縺阪ｋ縺玖ｩｦ縺・
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 謗｡逕ｨ縺励◆繧｢繝繝励ち繝ｼ縺ｧ繝・ヰ繧､繧ｹ繧剃ｽ懈・
		HRESULT hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		// 謖・ｮ壹＠縺滓ｩ溯・繝ｬ繝吶Ν縺ｧ繝・ヰ繧､繧ｹ縺檎函謌舌〒縺阪◆縺九ｒ遒ｺ隱・
		if (SUCCEEDED(hr)) {
			// 逕滓・縺ｧ縺阪◆縺ｮ縺ｧ繝ｭ繧ｰ蜃ｺ蜉帙＠縺ｦ繝ｫ繝ｼ繝苓┳蜃ｺ
#ifdef _DEBUG
			DebugLog(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
#endif // _DEBUG
			break;
		}
	}
	// 繝・ヰ繧､繧ｹ逕滓・縺御ｸ頑焔縺上＞縺九↑縺九▲縺溘・縺ｧ襍ｷ蜍輔〒縺阪↑縺・
	assert(device_ != nullptr);

#ifdef _DEBUG
	DebugLog("Complete create D3D12Device");
#endif // _DEBUG

	// 繧ｨ繝ｩ繝ｼ關ｽ縺｡蜃ｦ逅・
#ifdef _DEBUG
	DebugLog("---EnebleBreakOnSeverity---");
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		// 繝､繝舌お繝ｩ繝ｼ關ｽ縺｡
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		DebugLog("EnebleBreakOnSeverity_CORRUPTION");
		// 繧ｨ繝ｩ繝ｼ關ｽ縺｡
		if (disableError_) {
			DebugLog("!!! DisableBreakOnSeverity_ERROR !!!");
		} else {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			DebugLog("EnebleBreakOnSeverity_ERROR");
		}
		// 隴ｦ蜻・
		if (disableWarning_) {
			DebugLog("!!! DisableBreakOnSeverity_WARNING !!!");
		} else {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			DebugLog("EnebleBreakOnSeverity_WARNING");
		}

		// 繧ｨ繝ｩ繝ｼ謚大宛
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 謚大宛繝ｬ繝吶Ν
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 謖・ｮ壹Γ繝・そ繝ｼ繧ｸ繧呈椛蛻ｶ
		infoQueue->PushStorageFilter(&filter);

		// 隗｣謾ｾ
		infoQueue->Release();
	}
#endif // _DEBUG
}
