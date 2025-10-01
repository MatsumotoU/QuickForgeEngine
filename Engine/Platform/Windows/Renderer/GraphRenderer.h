#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Utility/DesignPatterns/Singleton.h"
#include "Graphic/ShaderBuffer/Data/PrimitiveVertexData.h"
#include "Graphic/ShaderBuffer/Data/Material.h"
#include "Graphic/ShaderBuffer/Data/TransformationMatrix.h"

#include "Graphic/ShaderBuffer/ConstantBuffer.h"

class PipelineStateObject;

// グラフ描画の最大数
static inline const uint32_t kGraphRendererMaxTriangleCount = 128;
static inline const uint32_t kGraphRendererMaxLineCount = 1024;
static inline const uint32_t kGraphRendererMaxPointCount = 128;

class GraphRenderer : public Singleton<GraphRenderer> {
public:
	friend class Singleton<GraphRenderer>;
	~GraphRenderer() override = default;

public:// 一回は呼び出さないとバグるやつら
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="engineCore"></param>
	void Initialize();
	/// <summary>
	/// 描画前準備
	/// </summary>
	void PreDraw();
	/// <summary>
	/// 描画コマンドを発行します
	/// </summary>
	void PostDraw();
	/// <summary>
	/// リソースを解放します
	/// </summary>
	void Finalize();

public:// 描画関数
	/// 三角形を描画します
	void DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, const Vector4& color);
	/// 線分を描画します
	void DrawLine(Vector3 point1, Vector3 point2,const Vector4& color);
	/// 点を描画します
	void DrawPoint(Vector3 point, const Vector4& color);
	/// グリッドを描画します
	void DrawGrid(float size = 10.0f, int32_t gridCount = 10);
	/// 球を描画します
	void DrawSphere(Vector3 center, float radius, const Vector4& color, uint32_t subdivision = 10);
	/// 円を描画します
	void DrawCircle(Vector3 center, float radius, const Vector4& color, uint32_t subdivision = 10);

	/// 何回三角形を描画するかを取得します
	void GetDrawTriangleCount(uint32_t* triangleCount) { *triangleCount = triangleCount_; };
	/// 何回ラインを描画するかを取得します
	void GetDrawLineCount(uint32_t* lineCount) { *lineCount = lineCount_; };
	/// 何回点を描画するかを取得します
	void GetDrawPointCount(uint32_t* pointCount) { *pointCount = pointCount_; };

private:
	uint32_t triangleCount_;
	uint32_t lineCount_;
	uint32_t pointCount_;

private:
	ConstantBuffer<TransformationMatrix> wvp_;
	ConstantBuffer<Material> material_;

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
	PipelineStateObject* trianglePso_;
	PipelineStateObject* linePso_;
	PipelineStateObject* pointPso_;
};