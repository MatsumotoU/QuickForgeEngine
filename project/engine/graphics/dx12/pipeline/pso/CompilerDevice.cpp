#include "CompilerDevice.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#include <cassert>
#include <format>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void CompilerDevice::Initialize() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create DxcUtils instance.", SystemError::Abort);
	}
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create DxcCompiler instance.", SystemError::Abort);
	}
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create default include handler.", SystemError::Abort);
	}

	createdDevice_ = true;
}

void CompilerDevice::Finalize() {
	if (includeHandler_) {
		includeHandler_->Release();
		includeHandler_ = nullptr;
	}
	if (dxcCompiler_) {
		dxcCompiler_->Release();
		dxcCompiler_ = nullptr;
	}
	if (dxcUtils_) {
		dxcUtils_->Release();
		dxcUtils_ = nullptr;
	}

	createdDevice_ = false;
}

IDxcUtils* QFE::GRAPHIC::INTERNAL::CompilerDevice::GetDxcUtils() const {
	CheckDeviceCreated();
	return dxcUtils_;
}

IDxcCompiler3* QFE::GRAPHIC::INTERNAL::CompilerDevice::GetDxcCompiler() const {
	CheckDeviceCreated();
	return dxcCompiler_;
}

IDxcIncludeHandler* QFE::GRAPHIC::INTERNAL::CompilerDevice::GetIncludeHandler() const {
	CheckDeviceCreated();
	return includeHandler_;
}

void QFE::GRAPHIC::INTERNAL::CompilerDevice::CheckDeviceCreated() const {
	if (!createdDevice_) {
		QFE_REPORT_SYSTEM_ERROR("CompilerDevice is not initialized. Call Initialize() before using.", SystemError::Abort);
	}
}
