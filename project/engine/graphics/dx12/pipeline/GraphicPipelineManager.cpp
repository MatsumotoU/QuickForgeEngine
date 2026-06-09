/**
 * @file GraphicPipelineManager.cpp
 * @brief グラフィックスパイプラインおよびルートシグネチャの管理クラスの実装
 */
#include "GraphicPipelineManager.h"
#include "file/FileUtility.h"
#include "string/MyString.h"

using namespace QFE::GRAPHIC::INTERNAL;
 /**
  * @brief 各種パイプラインの初期化
  * @param device ID3D12Deviceへのポインタ
  */
namespace {
	const std::string kVSFilePath = "engine/resources/shaders/vs/";
	const std::string kPSFilePath = "engine/resources/shaders/ps/";

	const std::string kObject3d = "Object3d";
}

void QFE::GRAPHIC::INTERNAL::GraphicPipelineManager::Initialize(
	std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc) {	
	// シェーダーを格納しているディレクトリ内のファイル名一覧を取得
	std::vector<std::string> vsFiles = QFE::FILE::GetFilesInDirectory(kVSFilePath);
	std::vector<std::string> psFiles = QFE::FILE::GetFilesInDirectory(kPSFilePath);
	// バイナリの一覧
	std::map<std::string, IDxcBlob*> vsBlobMap;
	std::map<std::string, IDxcBlob*> psBlobMap;

	// エンジンの付属シェーダーをコンパイル
	for (const auto& vsFile : vsFiles) {
		vsBlobMap[vsFile] = compileFunc(ConvertString(kVSFilePath + vsFile), L"vs_6_0");
	}
	for (const auto& psFile : psFiles) {
		psBlobMap[psFile] = compileFunc(ConvertString(kPSFilePath + psFile), L"ps_6_0");
	}

	// シェーダーのリフレクションを行うための関数群
	ShaderPairFunctions funcs;
	funcs.reflectionFunc = [&](IDxcBlob* shaderBlob) { shaderReflection_.RunShaderReflection(shaderBlob); };
	funcs.getInputLayoutFunc = [&](IDxcBlob* shaderBlob) { return shaderReflection_.GetInputLayoutElement(); };
	funcs.getRootParameterFunc = [&](IDxcBlob* shaderBlob) { return shaderReflection_.GetRootParameterElement(); };

	// エンジンの付属シェーダーペアを生成
	shaderPairs_[kObject3d].Create(vsBlobMap["Object3d.VS.hlsl"], psBlobMap["Object3d.PS.hlsl"], funcs);

}

void GraphicPipelineManager::Finalize() {
	
}