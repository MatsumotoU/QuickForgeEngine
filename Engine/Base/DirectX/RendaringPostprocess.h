#pragma once
class EngineCore;
#include "Sprite/Sprite.h"
#include "Object/OffsetBuffer.h"

#include <functional>
#include <vector>
#include <map>
#include <wrl.h>

// TODO: 他の効果を作成する
// TODO: ポストプロセスの順番を動的に変えられるようにする
// TODO: ポストプロセスのシェーダーを動的に作成できるようにする

class RendaringPostprosecess {
public:
	RendaringPostprosecess();

public:
	void Initialize(EngineCore* engineCore);

public:
	void PreDraw();
	void PostDraw();

public:// debug機能
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

private:
	void ClearFirstRenderTarget();
	void ClearSecondRenderTarget();
	void SwitchRenderTarget();

	void ApplyGrayScale();
	void ApplyBloom();

public:// パブリック変数
	bool isPostprocess_;

	bool enableGrayscale_;
	bool enableBloom_;

private:// グレースケール変数
	PipelineStateObject* grayScalePso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> grayScaleResource_;
	OffsetBuffer* grayScaleOffsetBuffer_;
	float grayScaleOffset_;

private:// メンバ変数
	std::vector<std::function<void() >> postProcessFunctions_;
	std::vector<uint32_t> postProcessOrderForm_;

	EngineCore* engineCore_;
	Sprite offscreen_;

	uint32_t postProcessCount_;
	uint32_t renderingRosourceIndex_;
	uint32_t readingResourceIndex_;
	bool isFirstStateRenderTarget_;
	bool isSecondStateRenderTarget_;

private:// 画面用
	float offScreenClearColor[4];
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;
};