#pragma once
#include <string>
#include "Math/Vector/Vector2.h"

#include "FontLoader.h"
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"
#include "Base/DirectX/WVPResource.h"
#include "Base/DirectX/Descriptors/DescriptorHandles.h"
#include "Math/Transform.h"

class EngineCore;
class Camera;

// TODO: パーティクルDrawCall
// TODO: uvをvsから送る

class Font final {
public:
	Font() = delete;
	Font(EngineCore* engineCore) : engineCore_(engineCore) {};

public:
	/// <summary>
	/// フォントの初期化
	/// </summary>
	/// <param name="fontFilePath">フォントファイルのパス</param>
	/// <param name="fontSize">フォントサイズ</param>
	void Initialize(const std::string& fontFilePath, float fontSize);
	/// <summary>
	/// フォントの描画
	/// </summary>
	/// <param name="text">描画するテキスト</param>
	/// <param name="position">描画位置</param>
	void Draw(const char& text, Camera* camera);

private:
	EngineCore* engineCore_ = nullptr;

	WVPResource wvpResource_;

	FontAtlasData fontAtlasData_;
	ConstantBuffer<FontDataToShader> fontDataBuffer_;
	VertexBuffer<PrimitiveVertexData> vertexBuffer_;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	uint32_t textureHandle_;
	const uint32_t kIncetanceCount_ = 10;

	Transform transform[10];
	DescriptorHandles instancingSrvHandles_;
};