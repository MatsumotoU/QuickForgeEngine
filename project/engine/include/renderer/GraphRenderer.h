/**
 * @file GraphRenderer.h
 * @brief プリミティブ（線、点、三角形等）の描画を行うクラス
 */

#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include "engine/include/graphic/ShaderBuffer/ConstantBuffer.h"

class PipelineStateObject;

// グラフ描画の最大数
static inline const uint32_t kGraphRendererMaxTriangleCount = 128;
static inline const uint32_t kGraphRendererMaxLineCount = 1024;
static inline const uint32_t kGraphRendererMaxPointCount = 128;

/**
 * @class GraphRenderer
 * @brief デバッグ用グリッド、ボックス、球体などのプリミティブ描画を管理するシングルトンクラス
 */
class GraphRenderer : public Singleton<GraphRenderer> {
public:
	friend class Singleton<GraphRenderer>;
	~GraphRenderer() override = default;

public:
    /** @brief 初期化処理 */
	void Initialize();
    /** @brief 描画前準備 */
	void PreDraw();
    /** @brief 描画コマンドの発行 */
	void PostDraw();
    /** @brief 終了処理・リソース解放 */
	void Finalize();

public:
	/** @brief 三角形を描画 */
	void DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, const Vector4& color);
	/** @brief 線を描画 */
	void DrawLine(Vector3 point1, Vector3 point2,const Vector4& color);
	/** @brief 点を描画 */
	void DrawPoint(Vector3 point, const Vector4& color);
	/** @brief グリッドを描画 */
	void DrawGrid(float size = 10.0f, int32_t gridCount = 10);
	/** @brief 球体を描画 */
	void DrawSphere(Vector3 center, float radius, const Vector4& color, uint32_t subdivision = 10);
	/** @brief 円を描画 */
	void DrawCircle(Vector3 center, float radius, const Vector4& color, uint32_t subdivision = 10);
	/** @brief 箱を描画 */
	void DrawBox(Vector3 min, Vector3 max, const Vector4& color);

    /** @brief 三角形の描画回数を取得 */
	void GetDrawTriangleCount(uint32_t* triangleCount) { *triangleCount = triangleCount_; };
    /** @brief ラインの描画回数を取得 */
	void GetDrawLineCount(uint32_t* lineCount) { *lineCount = lineCount_; };
    /** @brief 点の描画回数を取得 */
	void GetDrawPointCount(uint32_t* pointCount) { *pointCount = pointCount_; };

private:
	uint32_t triangleCount_;
	uint32_t lineCount_;
	uint32_t pointCount_;

private:
	ConstantBuffer<TransformationMatrix> wvp_;
	ConstantBuffer<Material> material_;

private:
	// 三角形の頂点チE�Eタ
	PrimitiveVertexData* triangleVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> triangleVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW triangleVertexBufferView_;
	// 線�E頂点チE�Eタ
	PrimitiveVertexData* lineVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> lineVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW lineVertexBufferView_;
	// 点の頂点チE�Eタ
	PrimitiveVertexData* pointVertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW pointVertexBufferView_;

private:
	PipelineStateObject* trianglePso_;
	PipelineStateObject* linePso_;
	PipelineStateObject* pointPso_;
};
