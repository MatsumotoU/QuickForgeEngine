#include "ShaderPair.h"
#include "ShaderReflection.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void ShaderPair::Create(IDxcBlob* vsBlob, IDxcBlob* psBlob) {
	// シェーダーのリフレクションを使用して、頂点シェーダーからInputLayoutを生成する
	ShaderReflection shaderReflection;
	shaderReflection.RunShaderReflection(vsBlob);

	// 頂点シェーダーの入力要素を取得
	inputLayout_.Initialize();
	std::vector<InputElement> inputElements = shaderReflection.GetInputLayoutElement();
	for (const auto& element : inputElements) {
		inputLayout_.CreateInputElementDesc(element);
	}
}
