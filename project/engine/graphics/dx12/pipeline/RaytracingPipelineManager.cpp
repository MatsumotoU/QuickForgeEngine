#include "RaytracingPipelineManager.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void RaytracingPipelineManager::Initialize(const RaytracingPipelineManagerInitializeInfo& initializeInfo) {
	// 初期化情報をメンバ変数に設定
	compileFunc = initializeInfo.compileFunc;
	getRootParameterFunc_ = initializeInfo.getRootParameterFunc;
	device_ = initializeInfo.device;

	// サポートされてるか確認
	if(device_ == nullptr) {
		QFE_LOG("RaytracingPipelineManager: Device5 is not available.");
		return;
	}

	// 初期化完了フラグを設定
	isActive_ = true;
}

void RaytracingPipelineManager::Finalize() {
	if (isActive_) {
		isActive_ = false;
	}
}

RTPSOHandle RaytracingPipelineManager::CreateRaytracingPipelineStateObject(const std::wstring& shaderFilePath, const wchar_t* profile) {
	// RaytracingPipelineManagerが有効かどうかを確認
	if (!CheckActive()) {
		return RTPSOHandle::Invalid;
	}

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
	raytracingPSO->CreatePipelineStateObject(shaderBlob, *(raytracingPSO->GetRootParameter().GetDescriptionRootSignature()), device_);
	// ShaderTableの作成
	raytracingPSO->CreateShaderTables(device_);

	// RaytracingPSOを管理するSparseSetに追加
	RTPSOHandle handle = static_cast<RTPSOHandle>(raytracingPSOs_.push_back(std::move(raytracingPSO)));

	return handle;
}

RaytracingPSO* QFE::GRAPHIC::RaytracingPipelineManager::GetRaytracingPipelineStateObject(RTPSOHandle handle) {
	return raytracingPSOs_.at(static_cast<uint32_t>(handle)).get();
}

bool QFE::GRAPHIC::RaytracingPipelineManager::CheckActive() const {
	if (!isActive_) {
		QFE_LOG("RaytracingPipelineManager: RaytracingPipelineManager is not active.");
		return false;
	}
	return true;
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
