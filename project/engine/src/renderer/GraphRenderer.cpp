/**
 * @file GraphRenderer.cpp
 * @brief デバッグ用図形描画クラスの実装
 */

#include "engine/include/renderer/GraphRenderer.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include <cassert>
#include <numbers>

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

/** @brief 初期化 */
void GraphRenderer::Initialize() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	GraphicPipelineManager* pipelineManager = GraphicPipelineManager::GetInstance();

	trianglePso_ = pipelineManager->GetPrimitivePso(kBlendModeNormal);
	linePso_ = pipelineManager->GetLinePso(kBlendModeNormal);
	pointPso_ = pipelineManager->GetPointPso(kBlendModeNormal);

	// 三角形の頂点リソースを作成
	triangleVertexResource_ = BufferGenerator::Generate(dxCommon->GetDevice(), sizeof(PrimitiveVertexData) * 3 * kGraphRendererMaxTriangleCount);
	triangleVertexBufferView_ = {};
	triangleVertexBufferView_.BufferLocation = triangleVertexResource_->GetGPUVirtualAddress();
	triangleVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * 3 * kGraphRendererMaxTriangleCount);
	triangleVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	triangleVertexData_ = nullptr;
	triangleVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&triangleVertexData_));

	// 線の頂点リソースを作成
	lineVertexResource_ = BufferGenerator::Generate(dxCommon->GetDevice(), sizeof(PrimitiveVertexData) * 2 * kGraphRendererMaxLineCount);
	lineVertexBufferView_ = {};
	lineVertexBufferView_.BufferLocation = lineVertexResource_->GetGPUVirtualAddress();
	lineVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * 2 * kGraphRendererMaxLineCount);
	lineVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	lineVertexData_ = nullptr;
	lineVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&lineVertexData_));

	// 点の頂点リソースを作成
	pointVertexResource_ = BufferGenerator::Generate(dxCommon->GetDevice(), sizeof(PrimitiveVertexData) * kGraphRendererMaxPointCount);
	pointVertexBufferView_ = {};
	pointVertexBufferView_.BufferLocation = pointVertexResource_->GetGPUVirtualAddress();
	pointVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * kGraphRendererMaxPointCount);
	pointVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	pointVertexData_ = nullptr;
	pointVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointVertexData_));

	wvp_.CreateResource(dxCommon->GetDevice());
	material_.CreateResource(dxCommon->GetDevice());

	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	material_.GetData()->enableLighting = 0;
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4();
}

/**
 * @brief 描画前処理
 * TODO: 毎フレーム最大数分バッファをループでクリアしているが、非常に非効率的。
 *       描画された分(count)だけクリアするか、そもそもクリアせず上書きするよう設計変更を推奨。
 */
void GraphRenderer::PreDraw() {
	CameraManager* cameraManager = CameraManager::GetInstance();
	Camera& camera = cameraManager->GetMainCamera();

	triangleCount_ = 0;
	lineCount_ = 0;
	pointCount_ = 0;

	// 頂点リソースをクリア
	for (uint32_t i = 0; i < kGraphRendererMaxTriangleCount; i++) {
		triangleVertexData_[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		triangleVertexData_[i].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		triangleVertexData_[i].texcoord = Vector2(0.0f, 0.0f);
	}
	for (uint32_t i = 0; i < kGraphRendererMaxLineCount; i++) {
		lineVertexData_[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		lineVertexData_[i].color = Vector4(1.0f, 1.0f, 1.0f,1.0f);
		lineVertexData_[i].texcoord = Vector2(0.0f, 0.0f);
	}
	for (uint32_t i = 0; i < kGraphRendererMaxPointCount; i++) {
		pointVertexData_[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		pointVertexData_[i].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		pointVertexData_[i].texcoord = Vector2(0.0f, 0.0f);
	}

	// カメラのワールドビュー投影行列を設定
	wvp_.GetData()->WVP = camera.GetWorldViewProjectionMatrix(Matrix4x4::MakeIndentity4x4(),CameraType::Perspective);
}

/** @brief 描画実行 */
void GraphRenderer::PostDraw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	if (triangleCount_ == 0 && lineCount_ == 0 && pointCount_ == 0) {
		return; // 描画するものが無い場合は何もしない
	}

	if (triangleCount_ > kGraphRendererMaxTriangleCount ||
		lineCount_ > kGraphRendererMaxLineCount ||
		pointCount_ > kGraphRendererMaxPointCount) {
		
		assert(false && "GraphRenderer: Exceeded maximum count of triangles, lines, or points.");
	}

	// 頂点リソースをGPUに転送
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

	if (triangleCount_ > 0) {
		commandList->RSSetViewports(1, dxCommon->GetViewPort());
		commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());

		commandList->SetGraphicsRootSignature(trianglePso_->GetRootSignature());
		commandList->SetPipelineState(trianglePso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &triangleVertexBufferView_);
		commandList->DrawInstanced(triangleCount_ * 3, 1, 0, 0);
	}
	if (lineCount_ > 0) {
		commandList->RSSetViewports(1, dxCommon->GetViewPort());
		commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());

		commandList->SetGraphicsRootSignature(linePso_->GetRootSignature());
		commandList->SetPipelineState(linePso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->IASetVertexBuffers(0, 1, &lineVertexBufferView_);
		commandList->DrawInstanced(lineCount_ * 2, 1, 0, 0);
	}
	if (pointCount_ > 0) {
		commandList->RSSetViewports(1, dxCommon->GetViewPort());
		commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());

		commandList->SetGraphicsRootSignature(pointPso_->GetRootSignature());
		commandList->SetPipelineState(pointPso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->IASetVertexBuffers(0, 1, &pointVertexBufferView_);
		commandList->DrawInstanced(pointCount_, 1, 0, 0);
	}
}

void GraphRenderer::Finalize() {
	
}

void GraphRenderer::DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, const Vector4& color) {
	if (triangleCount_ >= kGraphRendererMaxTriangleCount) {
#ifdef _DEBUG
		DebugLog("Exceeded maximum triangle count.");
#endif // _DEBUG
		return; // 譛螟ｧ謨ｰ繧定ｶ・∴縺溷ｴ蜷医・謠冗判縺励↑縺・
	}

	Vector4 p0 = Vector4(point1.x, point1.y, point1.z, 1.0f);
	Vector4 p1 = Vector4(point2.x, point2.y, point2.z, 1.0f);
	Vector4 p2 = Vector4(point3.x, point3.y, point3.z, 1.0f);

	Vector3 normalZ = { 0.0f, 0.0f, -1.0f };

	// 鬆らせ繝・・繧ｿ繧定ｨｭ螳・
	triangleVertexData_[triangleCount_ * 3 + 0].position = p0;
	triangleVertexData_[triangleCount_ * 3 + 0].color = color;
	triangleVertexData_[triangleCount_ * 3 + 0].texcoord = Vector2(0.0f, 0.0f);
	triangleVertexData_[triangleCount_ * 3 + 1].position = p1;
	triangleVertexData_[triangleCount_ * 3 + 1].color = color;
	triangleVertexData_[triangleCount_ * 3 + 1].texcoord = Vector2(0.0f, 0.0f);
	triangleVertexData_[triangleCount_ * 3 + 2].position = p2;
	triangleVertexData_[triangleCount_ * 3 + 2].color = color;
	triangleVertexData_[triangleCount_ * 3 + 2].texcoord = Vector2(0.0f, 0.0f);
	triangleCount_++;
	return;
}

void GraphRenderer::DrawLine(Vector3 point1, Vector3 point2, const Vector4& color) {
	if (lineCount_ >= kGraphRendererMaxLineCount) {
#ifdef _DEBUG
		DebugLog("Exceeded maximum Line count.");
#endif // _DEBUG
		return; // 譛螟ｧ謨ｰ繧定ｶ・∴縺溷ｴ蜷医・謠冗判縺励↑縺・
	}
	Vector4 p0 = Vector4(point1.x, point1.y, point1.z, 1.0f);
	Vector4 p1 = Vector4(point2.x, point2.y, point2.z, 1.0f);
	// 鬆らせ繝・・繧ｿ繧定ｨｭ螳・
	lineVertexData_[lineCount_ * 2 + 0].position = p0;
	lineVertexData_[lineCount_ * 2 + 0].color = color;
	lineVertexData_[lineCount_ * 2 + 0].texcoord = Vector2(0.0f, 0.0f);
	lineVertexData_[lineCount_ * 2 + 1].position = p1;
	lineVertexData_[lineCount_ * 2 + 1].color = color;
	lineVertexData_[lineCount_ * 2 + 1].texcoord = Vector2(0.0f, 0.0f);
	lineCount_++;
	return;
}

void GraphRenderer::DrawPoint(Vector3 point, const Vector4& color) {
	if (pointCount_ >= kGraphRendererMaxPointCount) {
#ifdef _DEBUG
		DebugLog("Exceeded maximum Points count.");
#endif // _DEBUG
		return; // 譛螟ｧ謨ｰ繧定ｶ・∴縺溷ｴ蜷医・謠冗判縺励↑縺・
	}
	Vector4 p = Vector4(point.x, point.y, point.z, 1.0f);
	Vector3 normalZ = { 0.0f, 0.0f, 1.0f };
	// 鬆らせ繝・・繧ｿ繧定ｨｭ螳・
	pointVertexData_[pointCount_].position = p;
	pointVertexData_[pointCount_].color = color;
	pointVertexData_[pointCount_].texcoord = Vector2(0.0f, 0.0f);
	pointCount_++;
	return;
}

void GraphRenderer::DrawGrid(float size, int32_t gridCount) {
	if (gridCount <= 0 || size <= 0.0f) {
		return; // 繧ｰ繝ｪ繝・ラ謨ｰ縺・莉･荳九∪縺溘・蛛ｶ謨ｰ縺ｮ蝣ｴ蜷医・謠冗判縺励↑縺・
	}
	
	float halfSize = size / 2.0f;
	Vector4 color = { 0.5f,0.5f,0.5f, 1.0f };
	for (int32_t i = 0; i <= gridCount; i++) {
		float t = static_cast<float>(i) / gridCount;
		float x = -halfSize + t * size;
		float z = -halfSize + t * size;

		float colorXt = (x + halfSize) / size;
		float colorZt = (z + halfSize) / size;

		if (i % 10 == 0) {
			color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		} else {
			color = Vector4::Leap(Vector4(0.5f, 0.5f, 0.5f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f), colorXt);
		}

		// 讓ｪ邱・
		if (x == 0.0f) {
			DrawLine(
				Vector3(x, 0.0f, -halfSize),
				Vector3(x, 0.0f, halfSize), Vector4(0.0f,0.0f,1.0f,1.0f));
		} else {
			DrawLine(
				Vector3(x, 0.0f, -halfSize),
				Vector3(x, 0.0f, halfSize), color);
		}
		
		if (i % 10 == 0) {
			color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		} else {
			color = color = Vector4::Leap(Vector4(0.5f, 0.5f, 0.5f, 0.1f), Vector4(0.0f, 0.0f, 1.0f, 1.0f), colorZt);
		}

		if (z == 0.0f) {
			// 邵ｦ邱・
			DrawLine(
				Vector3(-halfSize, 0.0f, z),
				Vector3(halfSize, 0.0f, z), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		} else {
			// 邵ｦ邱・
			DrawLine(
				Vector3(-halfSize, 0.0f, z),
				Vector3(halfSize, 0.0f, z), color);
		}
		
	}
}

void GraphRenderer::DrawSphere(Vector3 center, float radius, const Vector4& color, uint32_t subdivision) {
	const float pi = std::numbers::pi_v<float>;
	const float kLonEvery = pi / static_cast<float>(subdivision) * 2;
	const float kLatEvery = (pi * 2.0f) / static_cast<float>(subdivision) * 2;
	// 邱ｯ蠎ｦ縺ｮ譁ｹ蜷代↓蛻・牡
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * static_cast<float>(latIndex);// 迴ｾ蝨ｨ縺ｮ邱ｯ蠎ｦ
		float nextLat = (2.0f * pi) / static_cast<float>(subdivision) * 2.0f;

		// 邨悟ｺｦ縺ｮ譁ｹ蜷代↓蛻・牡
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lot = kLonEvery * static_cast<float>(lonIndex);// 迴ｾ蝨ｨ縺ｮ邱ｯ蠎ｦ
			float nextLot = pi / static_cast<float>(subdivision) * 2.0f;

			Vector3 a{}, b{}, c{};
			a = {
				cosf(lot) * cosf(lat),
				sinf(lot),
				cosf(lot) * sinf(lat) 
			};
			b = {
				cosf(lot + nextLot) * cosf(lat),
				sinf(lot + nextLot),
				cosf(lot + nextLot) * sinf(lat) 
			};
			c = {
				cosf(lot) * cosf(lat + nextLat),
				sinf(lot),
				cosf(lot) * sinf(lat + nextLat) 
			};

			// 蜊雁ｾ・・縺ｧ縺九￥縺吶ｋ
			a = a * radius;
			b = b * radius;
			c = c * radius;

			// 荳ｭ蠢・ｒ縺壹ｉ縺・
			a = a + center;
			b = b + center;
			c = c + center;

			DrawLine(a, b, color);
			DrawLine(a, c, color);
		}
	}
}

void GraphRenderer::DrawCircle(Vector3 center, float radius, const Vector4& color, uint32_t subdivision) {
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<Transform>(CameraManager::GetInstance()->GetMainCamera().GetBindEntityId())) {
		return;
	}
	Transform& camTransform = 
		entityManager->GetComponent<Transform>(CameraManager::GetInstance()->GetMainCamera().GetBindEntityId());

	Matrix4x4 matRot = Matrix4x4::MakeRotateXYZMatrix(camTransform.rotate);

	const float pi = std::numbers::pi_v<float>;
	const float kAngleEvery = (pi * 2.0f) / static_cast<float>(subdivision);
	Vector3 prevPoint = center + Vector3(radius, 0.0f, 0.0f);
	prevPoint = Vector3::Transform(prevPoint - center, matRot) + center;
	for (uint32_t i = 1; i <= subdivision; ++i) {
		float angle = kAngleEvery * static_cast<float>(i);
		Vector3 nextPoint = center + Vector3(cosf(angle) * radius, sinf(angle) * radius, 0.0f);
		nextPoint = Vector3::Transform(nextPoint - center, matRot) + center;
		DrawLine(prevPoint, nextPoint, color);
		prevPoint = nextPoint;
	}
}

void GraphRenderer::DrawBox(Vector3 min, Vector3 max, const Vector4& color) {
	// 8鬆らせ繧定ｨ育ｮ・
	Vector3 v0 = { min.x, min.y, min.z };
	Vector3 v1 = { max.x, min.y, min.z };
	Vector3 v2 = { max.x, max.y, min.z };
	Vector3 v3 = { min.x, max.y, min.z };
	Vector3 v4 = { min.x, min.y, max.z };
	Vector3 v5 = { max.x, min.y, max.z };
	Vector3 v6 = { max.x, max.y, max.z };
	Vector3 v7 = { min.x, max.y, max.z };
	// 12譛ｬ縺ｮ霎ｺ繧呈緒逕ｻ
	DrawLine(v0, v1, color); // 荳句燕
	DrawLine(v1, v2, color); // 蜿ｳ蜑・
	DrawLine(v2, v3, color); // 荳雁燕
	DrawLine(v3, v0, color); // 蟾ｦ蜑・
	DrawLine(v4, v5, color); // 荳句ｾ・
	DrawLine(v5, v6, color); // 蜿ｳ蠕・
	DrawLine(v6, v7, color); // 荳雁ｾ・
	DrawLine(v7, v4, color); // 蟾ｦ蠕・
	DrawLine(v0, v4, color); // 蟾ｦ荳・
	DrawLine(v1, v5, color); // 蜿ｳ荳・
	DrawLine(v2, v6, color); // 蜿ｳ荳・
	DrawLine(v3, v7, color); // 蟾ｦ荳・
}
