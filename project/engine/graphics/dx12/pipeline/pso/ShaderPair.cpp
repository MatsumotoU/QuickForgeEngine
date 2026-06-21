#include "ShaderPair.h"
#include "ShaderReflection.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void QFE::GRAPHIC::ShaderPair::Create(
	IDxcBlob* vsBlob, IDxcBlob* psBlob, const ShaderPairFunctions& funcs) {

	// シェーダーバイナリが有効かどうかを確認
	if (vsBlob == nullptr || psBlob == nullptr) {
		QFE_LOG("Invalid shader blob provided.");
		return;
	}

	// シェーダーのリフレクションを使用して、頂点シェーダーからInputLayoutを生成する
	funcs.reflectionFunc(vsBlob);

	// 頂点シェーダーの入力要素を取得
	inputLayout_.Initialize();
	std::vector<InputElement> inputElements = funcs.getInputLayoutFunc(vsBlob);
	for (const auto& element : inputElements) {
		inputLayout_.CreateInputElementDesc(element);
	}

	// ルートパラメーターの初期化
	rootParameter_.Initialize();
	// シェーダーのリフレクションを使用して、頂点シェーダーからRootParameterを生成する
	std::vector<RootParameterElement> rootParameterElements = funcs.getRootParameterFunc(vsBlob);
	for (const auto& element : rootParameterElements) {
		rootParameter_.CreateRootParameter(element, D3D12_SHADER_VISIBILITY_VERTEX);
	}
	// シェーダーのリフレクションを使用して、ピクセルシェーダーからRootParameterを生成する
	funcs.reflectionFunc(psBlob);
	std::vector<RootParameterElement> psRootParameterElements = funcs.getRootParameterFunc(psBlob);
	for (const auto& element : psRootParameterElements) {
		rootParameter_.CreateRootParameter(element, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	
	// 静的サンプラーの割り当て
	rootParameter_.AssignStaticSampler(funcs.getStaticSamplerFunc(), funcs.getStaticSamplerSizeFunc());

	// シェーダーバイナリを保存
	vsBlob_ = vsBlob;
	psBlob_ = psBlob;

	isCreated_ = true;
}
