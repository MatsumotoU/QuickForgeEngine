#pragma once
#include "math/MathInclude.h"

#define NOMINMAX
#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

struct VertexData;

namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
	enum class ShaderPairHandle : uint32_t;
	enum class DirectXResourceHandle : uint32_t;
	enum class BLASHandle : uint32_t;
}

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> CreateGraphicEngine(HWND hwnd);
	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine);

	/// @brief シェーダーペアを作成する関数
	bool CreateShaderPair(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::string& vsDirName, const std::string& psDirName,
		const std::string& vsFileName, const std::string& psFileName,
		QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle);

	/// @brief Object3d.GBuffer.PS用のルートリソースを作成する関数
	bool CreateObject3dGBufferRootResources(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle* rootResources,
		size_t rootResourcesSize);

	/// @brief Object3dのワールド行列とワールドビュー射影行列を更新する関数
	void UpdateObject3dWVPMatrix(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle,
		const QFE::MATH::Transform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix);

	/// @brief 頂点のデータから頂点位置を取得する関数
	std::vector<QFE::MATH::Vector3> GetModelVertexPositions(
		const VertexData* vertices, size_t vertexCount);

	/// @brief 頂点バッファを作成する関数
	bool CreateVertexBuffer(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<VertexData>& vertexPositions,
		const std::string& meshName,
		QFE::GRAPHIC::DirectXResourceHandle& outVertexBufferHandle);

	/// @brief BLASを作成する関数
	bool CreateBLAS(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::vector<VertexData>& vertices,
		const std::string& name,
		QFE::GRAPHIC::BLASHandle& outBLASHandle);
}