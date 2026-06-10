/**
 * @file GraphicPipelineManager.cpp
 * @brief グラフィックスパイプラインおよびルートシグネチャの管理クラスの実装
 */
#include "GraphicPipelineManager.h"
#include "file/FileUtility.h"
#include "string/MyString.h"

#include "pso/ShaderPair.h"
#include "pso/StaticSamplers.h"
#include "pso/ShaderReflection.h"
#include "pso/RasterizerTemplate.h"
#include "pso/BlendStates.h"
#include "pso/DepthStencilDescTemplate.h"
#include "pso/PipelineStateObject.h"

using namespace QFE::GRAPHIC::INTERNAL;
 /**
  * @brief 各種パイプラインの初期化
  * @param device ID3D12Deviceへのポインタ
  */
namespace {
	const std::string kVSFilePath = "engine/resources/shaders/vs/";
	const std::string kPSFilePath = "engine/resources/shaders/ps/";
}

QFE::GRAPHIC::INTERNAL::GraphicPipelineManager::GraphicPipelineManager() = default;

QFE::GRAPHIC::INTERNAL::GraphicPipelineManager::~GraphicPipelineManager() = default;

void GraphicPipelineManager::Initialize(std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc, ID3D12Device* device) {
	// 必要な機能のインスタンス生成
	shaderReflection_ = std::make_unique<ShaderReflection>();
	staticSamplers_ = std::make_unique<StaticSamplers>();
	rasterizerState_ = std::make_unique<RasterizerTemplate>();
	blendStates_ = std::make_unique<BlendStates>();
	depthStencilDescTemplate_ = std::make_unique<DepthStencilDescTemplate>();

	// 各機能の初期化
	staticSamplers_->Initialize();
	rasterizerState_->Initialize();
	blendStates_->Initialize();
	depthStencilDescTemplate_->Initialize();

	// シェーダーを格納しているディレクトリ内のファイル名一覧を取得
	std::vector<std::string> vsFiles = QFE::FILE::GetFilesInDirectory(kVSFilePath);
	std::vector<std::string> psFiles = QFE::FILE::GetFilesInDirectory(kPSFilePath);

	// * 既存のシェーダーペアの生成 * //
	// 基本設定
	ShaderPairElement element;
	element.vsDirName = kVSFilePath;
	element.psDirName = kPSFilePath;
	// MiniShader用のシェーダーペアの生成
	element.vsFileName = "MiniShader.VS.hlsl";
	element.psFileName = "MiniShader.PS.hlsl";
	GenerateShaderPair(element, compileFunc);

	// シェーダーペアからPSOを生成する
	GeneratePipelineStateObject(
		static_cast<ShaderPairHandle>(0), device, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		rasterizerState_->GetRasterizerDesc(RasterizerType::Default),
		blendStates_->GetBlendDesc(BlendMode::kBlendModeNormal),
		depthStencilDescTemplate_->GetDesc(DepthStencilDescType::Default));
}

void GraphicPipelineManager::Finalize() {
	
}

ShaderPairHandle QFE::GRAPHIC::INTERNAL::GraphicPipelineManager::GenerateShaderPair(
	const ShaderPairElement& element, std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc) {

	// バイナリの一覧(ファイル名をキーとして格納)
	std::map<std::string, IDxcBlob*> vsBlobMap;
	std::map<std::string, IDxcBlob*> psBlobMap;

	// シェーダーをコンパイル
	vsBlobMap[element.vsFileName] = compileFunc(ConvertString(element.vsDirName + element.vsFileName), L"vs_6_0");
	psBlobMap[element.psFileName] = compileFunc(ConvertString(element.psDirName + element.psFileName), L"ps_6_0");

	// シェーダーのリフレクションを行うための関数群
	ShaderPairFunctions funcs;
	funcs.reflectionFunc = [&](IDxcBlob* shaderBlob) { shaderReflection_->RunShaderReflection(shaderBlob); };
	funcs.getInputLayoutFunc = [&](IDxcBlob* shaderBlob) { return shaderReflection_->GetInputLayoutElement(); };
	funcs.getRootParameterFunc = [&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRootParameterElement(); };
	funcs.getStaticSamplerFunc = [&]() { return staticSamplers_->GetSamplerDescs(); };
	funcs.getStaticSamplerSizeFunc = [&]() { return staticSamplers_->GetSamplerCount(); };

	// シェーダーペアの生成
	shaderPairs_[shaderPairKeyCounter_] = std::make_unique<ShaderPair>();
	shaderPairs_[shaderPairKeyCounter_]->Create(vsBlobMap[element.vsFileName], psBlobMap[element.psFileName], funcs);
	return static_cast<ShaderPairHandle>(shaderPairKeyCounter_++);
}

PipelineStateObjectHandle QFE::GRAPHIC::INTERNAL::GraphicPipelineManager::GeneratePipelineStateObject(
	const ShaderPairHandle& shaderHandle, ID3D12Device* device
	, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, D3D12_RASTERIZER_DESC rasterizerDesc,
	D3D12_BLEND_DESC blendDesc, D3D12_DEPTH_STENCIL_DESC depthStencilDesc) {

	PipelineStateObjectElement element{};

	element.rootParameter = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetRootSignatureDesc();
	element.inputLayoutDesc = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetInputLayoutDesc();
	element.psBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetPSBlob();
	element.vsBlob = shaderPairs_[static_cast<uint32_t>(shaderHandle)]->GetVSBlob();
	element.topologyType = topologyType;
	element.rasterizerDesc = rasterizerDesc;
	element.blendDesc = blendDesc;
	element.depthStencilDesc = depthStencilDesc;

	// PSOの生成
	pipelineStateObjects_[pipelineStateObjectKeyCounter_] = std::make_unique<PipelineStateObject>();
	pipelineStateObjects_[pipelineStateObjectKeyCounter_]->CreatePipelineStateObject(element, device);

	return static_cast<PipelineStateObjectHandle>(pipelineStateObjectKeyCounter_++);
}