#pragma once
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"

#define NOMINMAX
#include <Windows.h>
#include <memory>

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> CreateGraphicEngine(HWND hwnd);
	/// @brief GUIマネージャを生成する関数.グラフィックエンジンとウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GUI::D3D12GuiManager> CreateGuiManager(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd);

	/// @brief シェーダーペアを作成する関数
	QFE::GRAPHIC::ShaderPairHandle CreateShaderPair(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		const std::string& vsDirName, const std::string& psDirName,
		const std::string& vsFileName, const std::string& psFileName);

	/// @brief Object3d.GBuffer.PS用のルートリソースを作成する関数
	void CreateObject3dGBufferRootResources(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources);

	/// @brief Object3dのワールド行列とワールドビュー射影行列を更新する関数
	void UpdateObject3dWVPMatrix(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
		QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle,
		const QFE::MATH::Transform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix);

	// 頂点のデータから頂点位置を取得する関数
	std::vector<QFE::MATH::Vector3> GetModelVertexPositions(
		const std::vector<VertexData>& vertices);
}