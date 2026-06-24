#include "RaytracingPipelineManager.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void RaytracingPipelineManager::Initialize(const RaytracingPipelineManagerInitializeInfo& initializeInfo) {
	// デバイスを変換して保持
	HRESULT hr = initializeInfo.device->QueryInterface(IID_PPV_ARGS(&device_));
	if(FAILED(hr)) {
		QFE_LOG("RaytracingPipelineManager: Failed to query ID3D12Device5 interface from ID3D12Device.");
		return;
	}

	compileFunc = initializeInfo.compileFunc;
	getRootParameterFunc_ = initializeInfo.getRootParameterFunc;

	isActive_ = true;
}

void RaytracingPipelineManager::Finalize() {
	if (isActive_) {
		device_.Reset();
		isActive_ = false;
	}
}

RTPSOHandle RaytracingPipelineManager::CreateRaytracingPipelineStateObject(const std::wstring& shaderFilePath, const wchar_t* profile) {
	// 必要な機能の確認
	if (!getRootParameterFunc_) {
		QFE_REPORT_SYSTEM_ERROR("RaytracingPipelineManager: getRootParameterFunc_ is not set.",SystemError::Abort);
		return RTPSOHandle::Invalid;
	}

	// レイトレーシングシェーダーをコンパイル
	IDxcBlob* shaderBlob = CompileRaytracingShader(shaderFilePath, profile);

	// RaytracingPSOのインスタンスを用意
	std::unique_ptr<RaytracingPSO> raytracingPSO = std::make_unique<RaytracingPSO>();

	// ルートパラメータの設定
	RootParameter& rootParam = raytracingPSO->GetRootParameter();
	std::vector<RootParameterElement> rootParams = getRootParameterFunc_(shaderBlob);
	for(RootParameterElement& param : rootParams) {
		rootParam.CreateRootParameter(param, D3D12_SHADER_VISIBILITY_ALL);
	}

	// RaytracingPSOを作成
	raytracingPSO->CreatePipelineStateObject(shaderBlob, *(raytracingPSO->GetRootParameter().GetDescriptionRootSignature()), device_.Get());

	// RaytracingPSOを管理するSparseSetに追加
	RTPSOHandle handle = static_cast<RTPSOHandle>(raytracingPSOs_.push_back(std::move(raytracingPSO)));

	return handle;
}

IDxcBlob* QFE::GRAPHIC::RaytracingPipelineManager::CompileRaytracingShader(const std::wstring& filePath, const wchar_t* profile) {
	// シェーダーのコンパイルを実行
	IDxcBlob* shaderBlob = nullptr;
	if (!compileFunc) {
		QFE_LOG("RaytracingPipelineManager: compileFunc is not set.");
		return shaderBlob;
	}

	// シェーダーのコンパイル関数を呼び出して、シェーダーをコンパイル
	shaderBlob = compileFunc(filePath, profile);

	// シェーダーのコンパイル結果を確認
	if(shaderBlob) {
		// シェーダーのコンパイルが成功した場合の処理
		QFE_LOG("RaytracingPipelineManager: Shader compiled successfully.");
		return shaderBlob;
	}
	return shaderBlob;
}
