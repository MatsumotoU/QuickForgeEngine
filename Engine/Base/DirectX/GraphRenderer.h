#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Math/PrimitiveVertexData.h"

#include "Base/DirectX/MaterialResource.h"
#include "Base/DirectX/WVPResource.h"

class EngineCore;
class PipelineStateObject;
class Camera;

// グラフ描画の最大数
static inline const uint32_t kGraphRendererMaxTriangleCount = 128;
static inline const uint32_t kGraphRendererMaxLineCount = 128;
static inline const uint32_t kGraphRendererMaxPointCount = 128;

class GraphRenderer {
public:// 一回は呼び出さないとバグるやつら
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="engineCore"></param>
	void Initialize(EngineCore* engineCore);
	/// <summary>
	/// 描画前準備
	/// </summary>
	void PreDraw();
	/// <summary>
	/// 描画コマンドを発行します
	/// </summary>
	void PostDraw();

public:// 描画関数
	/// <summary>
	/// 三角形を描画します(-1.0 ~ 1.0)
	/// </summary>
	void DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, const Vector4& color);
	/// <summary>
	/// 線分を描画します(-1.0 ~ 1.0)
	/// </summary>
	void DrawLine(Vector3 point1, Vector3 point2,const Vector4& color);
	/// <summary>
	/// 点を描画します(-1.0 ~ 1.0)
	/// </summary>
	void DrawPoint(Vector3 point, const Vector4& color);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="size"></param>
	/// <param name="gridCount"></param>
	void DrawGrid(float size = 10.0f, int32_t gridCount = 10);

public:// セッター
	void SetCamera(Camera* camera);

private:
	uint32_t triangleCount_;
	uint32_t lineCount_;
	uint32_t pointCount_;

private:
	WVPResource wvpResource_;
	MaterialResource materialResource_;

private:
	// 三角形の頂点データ
	PrimitiveVertexData* triangleVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> triangleVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW triangleVertexBufferView_;
	// 線の頂点データ
	PrimitiveVertexData* lineVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> lineVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW lineVertexBufferView_;
	// 点の頂点データ
	PrimitiveVertexData* pointVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW pointVertexBufferView_;

private:
	EngineCore* engineCore_;
	PipelineStateObject* trianglePso_;
	PipelineStateObject* linePso_;
	PipelineStateObject* pointPso_;

private:
	Camera* camera_; // 描画するカメラ
};