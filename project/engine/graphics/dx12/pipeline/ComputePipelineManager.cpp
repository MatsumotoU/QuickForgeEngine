#include "ComputePipelineManager.h"
#include "string/MyString.h"

using namespace QFE::GRAPHIC;

ComputePipelineManager::ComputePipelineManager() = default;

void ComputePipelineManager::Initialize(ComputePipelineManagerInitializeInfo initializeInfo) {
	initializeInfo_ = initializeInfo;
}

ComputePSOHandle ComputePipelineManager::GenerateComputePipelineStateObject(const std::string& csDirPath, const std::string& csFileName) {
	std::unique_ptr<ComputePSO> computePSO = std::make_unique<ComputePSO>();
	// シェーダーをコンパイル,コンパイル済みのバイナリを取得
	if(csBlobMap_.find(csFileName) == csBlobMap_.end()) {
		IDxcBlob* csBlob = initializeInfo_.compileFunc(ConvertString( csDirPath + csFileName), L"cs_6_0");
		csBlobMap_[csFileName] = csBlob;
	} else {
		QFE_LOG(std::format("Compute shader binary for {} already exists. Using cached version.", csFileName));
	}

	// ルートシグネチャの生成
	std::vector<RootParameterElement> rootParameters = initializeInfo_.getRootParameterFunc(csBlobMap_[csFileName]);
	computePSO->GetRootParameter().Initialize(D3D12_ROOT_SIGNATURE_FLAG_NONE);
	for(RootParameterElement& rootParameter : rootParameters) {
		computePSO->GetRootParameter().CreateRootParameter(rootParameter, D3D12_SHADER_VISIBILITY_ALL);
	}

	// ルートシグネチャの説明を取得
	D3D12_ROOT_SIGNATURE_DESC* rootSigDesc = computePSO->GetRootParameter().GetDescriptionRootSignature();

	// パイプラインステートオブジェクトの生成
	computePSO->CreatePipelineStateObject(csBlobMap_[csFileName], *rootSigDesc, initializeInfo_.device);

	// スレッドグループサイズの設定
	UINT sizeX, sizeY, sizeZ;
	initializeInfo_.getThreadGroupSizeFunc(csBlobMap_[csFileName], sizeX, sizeY, sizeZ);
	computePSO->SetThreadGroupSize(sizeX, sizeY, sizeZ);

	// ComputePSOを生成
	uint32_t handle = computePSOs_.push_back(nullptr); // 一時的にnullptrを追加して、後でunique_ptrに置き換える
	computePSOs_.at(handle) = std::move(computePSO); // unique_ptrに置き換える
	return static_cast<ComputePSOHandle>(handle);
}

ID3D12RootSignature* QFE::GRAPHIC::ComputePipelineManager::GetRootSignature(const ComputePSOHandle& handle) const {
	if(!computePSOs_.Contains(static_cast<uint32_t>(handle))) {
		QFE_LOG(std::format("ComputePSOHandle {} is invalid. Cannot retrieve root signature.", static_cast<uint32_t>(handle)));
		assert(false && "Invalid ComputePSOHandle.");
		return nullptr;
	}
	return computePSOs_.at(static_cast<uint32_t>(handle))->GetRootSignature();
}

ID3D12PipelineState* QFE::GRAPHIC::ComputePipelineManager::GetPipelineState(const ComputePSOHandle& handle) const {
	if(!computePSOs_.Contains(static_cast<uint32_t>(handle))) {
		QFE_LOG(std::format("ComputePSOHandle {} is invalid. Cannot retrieve pipeline state.", static_cast<uint32_t>(handle)));
		assert(false && "Invalid ComputePSOHandle.");
		return nullptr;
	}
	return computePSOs_.at(static_cast<uint32_t>(handle))->GetPipelineState();
}

std::vector<D3D12_ROOT_PARAMETER_TYPE> QFE::GRAPHIC::ComputePipelineManager::GetRootParameterTypes(const ComputePSOHandle& handle) const {
	if(!computePSOs_.Contains(static_cast<uint32_t>(handle))) {
		QFE_LOG(std::format("ComputePSOHandle {} is invalid. Cannot retrieve root parameter types.", static_cast<uint32_t>(handle)));
		assert(false && "Invalid ComputePSOHandle.");
		return {};
	}
	return computePSOs_.at(static_cast<uint32_t>(handle))->GetRootParameter().GetRootParameterTypes();
}

bool ComputePipelineManager::GetThreadGroupSize(const ComputePSOHandle& handle, UINT& sizeX, UINT& sizeY, UINT& sizeZ) const {
	if(!computePSOs_.Contains(static_cast<uint32_t>(handle))) {
		QFE_LOG(std::format("ComputePSOHandle {} is invalid. Cannot retrieve thread group size.", static_cast<uint32_t>(handle)));
		assert(false && "Invalid ComputePSOHandle.");
		return false;
	}

	computePSOs_.at(static_cast<uint32_t>(handle))->GetThreadGroupSize(sizeX, sizeY, sizeZ);
	return computePSOs_.at(static_cast<uint32_t>(handle))->IsThreadGroupSizeSet();
}
