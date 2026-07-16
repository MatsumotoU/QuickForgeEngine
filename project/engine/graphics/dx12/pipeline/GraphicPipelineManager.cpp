/**
 * @file GraphicPipelineManager.cpp
 * @brief グラフィックスパイプラインおよびルートシグネチャの管理クラスの実装
 */
#include "GraphicPipelineManager.h"
#include "file/FileUtility.h"
#include "string/MyString.h"

#include "pso/ShaderPair.h"
#include "pso/StaticSamplerTemplate.h"
#include "pso/ShaderReflection.h"
#include "pso/RasterizerTemplate.h"
#include "pso/BlendStateTemplate.h"
#include "pso/DepthStencilDescTemplate.h"


#include "EngineDefines.h"

using namespace QFE::GRAPHIC;
 /**
  * @brief 各種パイプラインの初期化
  * @param device ID3D12Deviceへのポインタ
  */
namespace {
	const std::string kVSFilePath = "engine/resources/shaders/vs/";
	const std::string kPSFilePath = "engine/resources/shaders/ps/";
}

QFE::GRAPHIC::GraphicPipelineManager::GraphicPipelineManager() = default;

QFE::GRAPHIC::GraphicPipelineManager::~GraphicPipelineManager() = default;

void GraphicPipelineManager::Initialize(GraphicPipelineManagerInitializeInfo initializeInfo) {
	// 必要な機能のインスタンス生成
	staticSamplers_ = std::make_unique<StaticSamplerTemplate>();
	rasterizerState_ = std::make_unique<RasterizerTemplate>();
	blendStates_ = std::make_unique<BlendStateTemplate>();
	depthStencilDescTemplate_ = std::make_unique<DepthStencilDescTemplate>();

	// 初期化情報を保持
	initializeInfo_ = initializeInfo;

	// 各機能の初期化
	staticSamplers_->Initialize();
	rasterizerState_->Initialize();
	blendStates_->Initialize();
	depthStencilDescTemplate_->Initialize();

	// シェーダーを格納しているディレクトリ内のファイル名一覧を取得
	std::vector<std::string> vsFiles = QFE::FILE::GetFilesInDirectory(kVSFilePath);
	std::vector<std::string> psFiles = QFE::FILE::GetFilesInDirectory(kPSFilePath);

	GenerateBuiltInShaderPairs();
	// BuiltInのPSOを生成
	GenerateBuiltInPSO(initializeInfo.device);
}

void GraphicPipelineManager::Finalize() {
	
}

ShaderPairHandle QFE::GRAPHIC::GraphicPipelineManager::GenerateShaderPair(const ShaderPairElement& element) {

	// すでに同じシェーダーペアが存在する場合は、既存のハンドルを返す
	if (shaderPairNameToKeyMap_.find(element.vsFileName + element.psFileName) != shaderPairNameToKeyMap_.end()) {
		return static_cast<ShaderPairHandle>(shaderPairNameToKeyMap_[element.vsFileName + element.psFileName]);
	}

	// バイナリの一覧(ファイル名をキーとして格納)
	std::map<std::string, IDxcBlob*> vsBlobMap;
	std::map<std::string, IDxcBlob*> psBlobMap;

	// シェーダーをコンパイル
	vsBlobMap[element.vsFileName] = initializeInfo_.compileFunc(ConvertString(element.vsDirName + element.vsFileName), L"vs_6_0");
	psBlobMap[element.psFileName] = initializeInfo_.compileFunc(ConvertString(element.psDirName + element.psFileName), L"ps_6_0");

	// シェーダーのリフレクションを行うための関数群
	ShaderPairFunctions funcs;
	funcs.getInputLayoutFunc = [&](IDxcBlob* shaderBlob) { return initializeInfo_.getInputLayoutFunc(shaderBlob); };
	funcs.getRootParameterFunc = [&](IDxcBlob* shaderBlob) { return initializeInfo_.getRootParameterFunc(shaderBlob); };
	funcs.getStaticSamplerFunc = [&]() { return staticSamplers_->GetSamplerDescs(); };
	funcs.getStaticSamplerSizeFunc = [&]() { return staticSamplers_->GetSamplerCount(); };
	funcs.getRenderTargetCountFunc = [&](IDxcBlob* shaderBlob) { return initializeInfo_.getRenderTargetCountFunc(shaderBlob); };

	// シェーダーペアの生成
	shaderPairs_[shaderPairKeyCounter_] = std::make_unique<ShaderPair>();
	shaderPairs_[shaderPairKeyCounter_]->Create(vsBlobMap[element.vsFileName], psBlobMap[element.psFileName], funcs);

	// シェーダーペアの名前とキーをマップに登録
	shaderPairNameToKeyMap_[element.vsFileName + element.psFileName] = shaderPairKeyCounter_;
	return static_cast<ShaderPairHandle>(shaderPairKeyCounter_++);
}

PSOHandle QFE::GRAPHIC::GraphicPipelineManager::GeneratePipelineStateObject(
	const ShaderPairHandle& shaderHandle, ID3D12Device* device
	, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, D3D12_RASTERIZER_DESC rasterizerDesc,
	D3D12_BLEND_DESC blendDesc, D3D12_DEPTH_STENCIL_DESC depthStencilDesc) {

	// PSOの一意性を保証するためのキーを生成
	std::string key = std::format("{}{}{}{}{}{}{}", 
		static_cast<uint32_t>(shaderHandle), static_cast<uint32_t>(topologyType),
		static_cast<uint32_t>(rasterizerDesc.CullMode), static_cast<uint32_t>(rasterizerDesc.FillMode),
		static_cast<uint32_t>(blendDesc.AlphaToCoverageEnable), static_cast<uint32_t>(depthStencilDesc.DepthEnable),
		static_cast<uint32_t>(depthStencilDesc.StencilEnable));
	// すでに同じPSOが存在する場合は、既存のハンドルを返す
	if (pipelineStateObjectNameToKeyMap_.find(key) != pipelineStateObjectNameToKeyMap_.end()) {
		return static_cast<PSOHandle>(pipelineStateObjectNameToKeyMap_[key]);
	}

	PipelineStateObjectElement element{};

	element.shaderPairHandle = static_cast<uint32_t>(shaderHandle);
	element.rootParameter = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetRootSignatureDesc();
	element.inputLayoutDesc = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetInputLayoutDesc();
	element.psBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetPSBlob();
	element.vsBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetVSBlob();
	element.topologyType = topologyType;
	element.rasterizerDesc = rasterizerDesc;
	element.blendDesc = blendDesc;
	element.depthStencilDesc = depthStencilDesc;
	element.numRenderTarget = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetRenderTargetCount();

	// PSOの生成
	pipelineStateObjects_[pipelineStateObjectKeyCounter_] = std::make_unique<PipelineStateObject>();
	pipelineStateObjects_[pipelineStateObjectKeyCounter_]->CreatePipelineStateObject(element, device);

	// PSOの名前とキーをマップに登録
	pipelineStateObjectNameToKeyMap_[key] = pipelineStateObjectKeyCounter_;

	return static_cast<PSOHandle>(pipelineStateObjectKeyCounter_++);
}

PSOHandle GraphicPipelineManager::GeneratePipelineStateObject(
	ID3D12Device* device, const ShaderPairHandle& shaderHandle, BlendMode blendMode,
	RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType) {

	PipelineStateObjectElement element{};
	element.shaderPairHandle = static_cast<uint32_t>(shaderHandle);
	element.rootParameter = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetRootSignatureDesc();
	element.inputLayoutDesc = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetInputLayoutDesc();
	element.psBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetPSBlob();
	element.vsBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetVSBlob();
	element.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	element.rasterizerDesc = rasterizerState_->GetRasterizerDesc(rasterizerType);
	element.blendDesc = blendStates_->GetBlendDesc(blendMode);
	element.depthStencilDesc = depthStencilDescTemplate_->GetDesc(depthStencilDescType);
	element.numRenderTarget = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetRenderTargetCount();

	// PSOの生成
	pipelineStateObjects_[pipelineStateObjectKeyCounter_] = std::make_unique<PipelineStateObject>();
	pipelineStateObjects_[pipelineStateObjectKeyCounter_]->CreatePipelineStateObject(element, device);
	return static_cast<PSOHandle>(pipelineStateObjectKeyCounter_++);
}

PipelineStateObject* GraphicPipelineManager::GetPipelineStateObject(const PSOHandle& psoHandle) const {
	if (pipelineStateObjects_.Contains(static_cast<uint32_t>(psoHandle))) {
		return pipelineStateObjects_.at(static_cast<uint32_t>(psoHandle)).get();
	}

	QFE_LOG(std::format("PipelineStateObject not found. PSOHandle: {}", static_cast<uint32_t>(psoHandle)));
	return nullptr;
}

ID3D12PipelineState* QFE::GRAPHIC::GraphicPipelineManager::GetPipelineState(const PSOHandle& psoHandle) const {
	PipelineStateObject* pso = GetPipelineStateObject(psoHandle);
	if(pso) {
		return pso->GetPipelineState();
	}

	QFE_LOG(std::format("PipelineState not found. PSOHandle: {}", static_cast<uint32_t>(psoHandle)));
	return nullptr;
}

ID3D12RootSignature* GraphicPipelineManager::GetRootSignature(const PSOHandle& psoHandle) const {
	PipelineStateObject* pso = GetPipelineStateObject(psoHandle);
	if(pso) {
		return pso->GetRootSignature();
	}
	QFE_LOG(std::format("RootSignature not found. PSOHandle: {}", static_cast<uint32_t>(psoHandle)));
	return nullptr;
}

std::vector<D3D12_ROOT_PARAMETER_TYPE> GraphicPipelineManager::GetRootParameterTypes(const PSOHandle& psoHandle) const {
	// PSOハンドルからPSOを取得し、そこからシェーダーペアのハンドルを取得して、シェーダーペアからルートパラメータのタイプを取得する
	PipelineStateObject* pso = GetPipelineStateObject(psoHandle);
	if (pso) {
		uint32_t shaderPairHandle = pso->GetShaderPairHandle();
		return shaderPairs_.at(shaderPairHandle)->GetRootParameterTypes();
	}
	QFE_REPORT_SYSTEM_ERROR(std::format("RootParameterTypes not found. PSOHandle: {}", static_cast<uint32_t>(psoHandle)),SystemError::Abort);
	return {};
}

PSOHandle GraphicPipelineManager::GetBuiltInPSOHandle(
	BuiltInShaderPair builtInShaderPair, BlendMode blendMode, 
	RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType) const {

	// BuiltInのPSOを管理する多次元配列から、引数で指定された情報をもとにPSOハンドルを取得する
	std::array<uint32_t, 4> infoArray = {
		static_cast<uint32_t>(builtInPairHandles_.at(builtInShaderPair)),
		static_cast<uint32_t>(blendMode),
		static_cast<uint32_t>(rasterizerType),
		static_cast<uint32_t>(depthStencilDescType)
	};
	return BuiltInPSOs_.At(infoArray);
}

void GraphicPipelineManager::GenerateBuiltInShaderPairs() {
	// 基本設定
	ShaderPairElement element;
	element.vsDirName = kVSFilePath;
	element.psDirName = kPSFilePath;

	// MiniShader用のシェーダーペアの生成
	element.vsFileName = "MiniShader.VS.hlsl";
	element.psFileName = "MiniShader.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::ObjectMini] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::ObjectMini]),
		element.vsFileName, element.psFileName));

	// Object2D用のシェーダーペアの生成
	element.vsFileName = "Object2d.VS.hlsl";
	element.psFileName = "Object2d.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::Object2D] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::Object2D]),
		element.vsFileName, element.psFileName));

	// Object3D用のシェーダーペアの生成
	element.vsFileName = "Object3d.VS.hlsl";
	element.psFileName = "Object3d.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::Object3D] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::Object3D]),
		element.vsFileName, element.psFileName));

	// Particle用のシェーダーペアの生成
	element.vsFileName = "Particle.VS.hlsl";
	element.psFileName = "Particle.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::Particle] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::Particle]),
		element.vsFileName, element.psFileName));

	// Primitive用のシェーダーペアの生成
	element.vsFileName = "Primitive.VS.hlsl";
	element.psFileName = "Primitive.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::Primitive] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::Primitive]),
		element.vsFileName, element.psFileName));

	// Skybox用のシェーダーペアの生成
	element.vsFileName = "Skybox.VS.hlsl";
	element.psFileName = "Skybox.PS.hlsl";
	builtInPairHandles_[BuiltInShaderPair::Skybox] = GenerateShaderPair(element);
	QFE_LOG(std::format("BuiltIn ShaderPair generated. ShaderPairHandle: {}, VS: {}, PS: {}",
		static_cast<uint32_t>(builtInPairHandles_[BuiltInShaderPair::Skybox]),
		element.vsFileName, element.psFileName));
}

void QFE::GRAPHIC::GraphicPipelineManager::GenerateBuiltInPSO(ID3D12Device* device) {

	// BuiltInのPSOを管理する多次元配列のサイズを設定するために、各情報の数を取得して配列に格納する
	std::array<size_t, 4> sizes = {
		static_cast<size_t>(builtInPairHandles_.size()),
		blendStates_->GetBlendDescMap().size(),
		rasterizerState_->GetRasterizerDescMap().size(),
		depthStencilDescTemplate_->GetDescMap().size()
	};
	BuiltInPSOs_.SetSize(sizes);

	// シェーダーペアからPSOを生成する
	for (const auto& [builtInShaderPair, shaderPairHandle] : builtInPairHandles_) {
		for(const auto& [blendMode, blendDesc] : blendStates_->GetBlendDescMap()) {
			for (const auto& [rasterizerType, rasterizerDesc] : rasterizerState_->GetRasterizerDescMap()) {
				for (const auto& [depthStencilDescType, depthStencilDesc] : depthStencilDescTemplate_->GetDescMap()) {
					// pairHandle,blend,rasterize,depthの順番で管理する多次元配列に格納するための情報をまとめる
					std::array<uint32_t, 4> infoArray = {
						static_cast<uint32_t>(shaderPairHandle),
						static_cast<uint32_t>(blendMode),
						static_cast<uint32_t>(rasterizerType),
						static_cast<uint32_t>(depthStencilDescType)
					};

					// PSOの生成
					BuiltInPSOs_.At(infoArray) = GeneratePipelineStateObject(
						shaderPairHandle, device, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
						rasterizerDesc, blendDesc, depthStencilDesc);

					QFE_LOG(std::format("BuiltIn PSO generated. ShaderPairHandle: {}, BlendMode: {}, RasterizerType: {}, DepthStencilDescType: {}",
						static_cast<uint32_t>(shaderPairHandle), static_cast<uint32_t>(blendMode),
						static_cast<uint32_t>(rasterizerType), static_cast<uint32_t>(depthStencilDescType)));
					QFE_LOG(std::format("PSOHandle: {}", static_cast<uint32_t>(BuiltInPSOs_.At(infoArray))));
				}
			}
		}
	}
}