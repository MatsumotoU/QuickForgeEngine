#include "GraphRenderer.h"
#include "Base/EngineCore.h"
#include "Camera/Camera.h"

#include <cassert>

void GraphRenderer::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	trianglePso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
	linePso_ = engineCore->GetGraphicsCommon()->GetLinePso(kBlendModeNormal);
	pointPso_ = engineCore->GetGraphicsCommon()->GetPointPso(kBlendModeNormal);

	// 三角形の頂点リソースを作成
	triangleVertexResource_ = CreateBufferResource(engineCore_->GetDirectXCommon()->GetDevice(), sizeof(PrimitiveVertexData) * 3 * kGraphRendererMaxTriangleCount);
	triangleVertexBufferView_ = {};
	triangleVertexBufferView_.BufferLocation = triangleVertexResource_->GetGPUVirtualAddress();
	triangleVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * 3 * kGraphRendererMaxTriangleCount);
	triangleVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	triangleVertexData_ = nullptr;
	triangleVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&triangleVertexData_));
	// 線の頂点リソースを作成
	lineVertexResource_ = CreateBufferResource(engineCore_->GetDirectXCommon()->GetDevice(), sizeof(PrimitiveVertexData) * 2 * kGraphRendererMaxLineCount);
	lineVertexBufferView_ = {};
	lineVertexBufferView_.BufferLocation = lineVertexResource_->GetGPUVirtualAddress();
	lineVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * 2 * kGraphRendererMaxLineCount);
	lineVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	lineVertexData_ = nullptr;
	lineVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&lineVertexData_));
	// 点の頂点リソースを作成
	pointVertexResource_ = CreateBufferResource(engineCore_->GetDirectXCommon()->GetDevice(), sizeof(PrimitiveVertexData) * kGraphRendererMaxPointCount);
	pointVertexBufferView_ = {};
	pointVertexBufferView_.BufferLocation = pointVertexResource_->GetGPUVirtualAddress();
	pointVertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(PrimitiveVertexData) * kGraphRendererMaxPointCount);
	pointVertexBufferView_.StrideInBytes = sizeof(PrimitiveVertexData);
	pointVertexData_ = nullptr;
	pointVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointVertexData_));

	wvpResource_.Initialize(engineCore_->GetDirectXCommon(), 1);
	materialResource_.Initialize(engineCore_->GetDirectXCommon());
	materialResource_.materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void GraphRenderer::PreDraw() {
	if (camera_ == nullptr) {
		return;
	}

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
	if (camera_) {
		assert(camera_);
		wvpResource_.SetWVPMatrix(camera_->MakeWorldViewProjectionMatrix(Matrix4x4::MakeIndentity4x4(), CAMERA_VIEW_STATE_PERSPECTIVE), 0);
	}
}

void GraphRenderer::PostDraw() {
	if (camera_ == nullptr) {
		return;
	}

	if (triangleCount_ == 0 && lineCount_ == 0 && pointCount_ == 0) {
		return; // 描画するものがない場合は何もしない
	}

	if (triangleCount_ > kGraphRendererMaxTriangleCount ||
		lineCount_ > kGraphRendererMaxLineCount ||
		pointCount_ > kGraphRendererMaxPointCount) {
		
		assert(false && "GraphRenderer: Exceeded maximum count of triangles, lines, or points.");
	}

	// 頂点リソースをGPUに転送
	ID3D12GraphicsCommandList* commandList = engineCore_->GetDirectXCommon()->GetCommandList();

	if (triangleCount_ > 0) {
		assert(camera_);
		commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
		commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());

		commandList->SetGraphicsRootSignature(trianglePso_->GetRootSignature());
		commandList->SetPipelineState(trianglePso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterial()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource_.GetWVPResource()->GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &triangleVertexBufferView_);
		commandList->DrawInstanced(triangleCount_ * 3, 1, 0, 0);
	}
	if (lineCount_ > 0) {
		assert(camera_);
		commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
		commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());

		commandList->SetGraphicsRootSignature(linePso_->GetRootSignature());
		commandList->SetPipelineState(linePso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterial()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource_.GetWVPResource()->GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->IASetVertexBuffers(0, 1, &lineVertexBufferView_);
		commandList->DrawInstanced(lineCount_ * 2, 1, 0, 0);
	}
	if (pointCount_ > 0) {
		assert(camera_);
		commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
		commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());

		commandList->SetGraphicsRootSignature(pointPso_->GetRootSignature());
		commandList->SetPipelineState(pointPso_->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_.GetMaterial()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource_.GetWVPResource()->GetGPUVirtualAddress());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->IASetVertexBuffers(0, 1, &pointVertexBufferView_);
		commandList->DrawInstanced(pointCount_, 1, 0, 0);
	}


}

void GraphRenderer::DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, const Vector4& color) {
	if (camera_ == nullptr) {
		return;
	}

	if (triangleCount_ >= kGraphRendererMaxTriangleCount) {
		return; // 最大数を超えた場合は描画しない
	}

	Vector4 p0 = Vector4(point1.x, point1.y, point1.z, 1.0f);
	Vector4 p1 = Vector4(point2.x, point2.y, point2.z, 1.0f);
	Vector4 p2 = Vector4(point3.x, point3.y, point3.z, 1.0f);

	Vector3 normalZ = { 0.0f, 0.0f, -1.0f };

	// 頂点データを設定
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
	if (camera_ == nullptr) {
		return;
	}

	if (lineCount_ >= kGraphRendererMaxLineCount) {
		return; // 最大数を超えた場合は描画しない
	}
	Vector4 p0 = Vector4(point1.x, point1.y, point1.z, 1.0f);
	Vector4 p1 = Vector4(point2.x, point2.y, point2.z, 1.0f);
	// 頂点データを設定
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
	if (camera_ == nullptr) {
		return;
	}

	if (pointCount_ >= kGraphRendererMaxPointCount) {
		return; // 最大数を超えた場合は描画しない
	}
	Vector4 p = Vector4(point.x, point.y, point.z, 1.0f);
	Vector3 normalZ = { 0.0f, 0.0f, 1.0f };
	// 頂点データを設定
	pointVertexData_[pointCount_].position = p;
	pointVertexData_[pointCount_].color = color;
	pointVertexData_[pointCount_].texcoord = Vector2(0.0f, 0.0f);
	pointCount_++;
	return;
}

void GraphRenderer::DrawGrid(float size, int32_t gridCount) {
	if (camera_ == nullptr) {
		return;
	}

	if (gridCount <= 0 || size <= 0.0f) {
#ifdef _DEBUG
		DebugLog("DrawGrid: gridCount = 0 || size <= 0");
#endif // _DEBUG
		return; // グリッド数が0以下または偶数の場合は描画しない
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
			color = Vector4::Leap(Vector4(0.5f, 0.5f, 0.5f, 0.5f), Vector4(1.0f, 0.3f, 0.3f, 1.0f), colorXt);
		}

		// 横線
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
			color = color = Vector4::Leap(Vector4(0.5f, 0.5f, 0.5f, 0.5f), Vector4(0.3f, 0.3f, 1.0f, 1.0f), colorZt);
		}

		if (z == 0.0f) {
			// 縦線
			DrawLine(
				Vector3(-halfSize, 0.0f, z),
				Vector3(halfSize, 0.0f, z), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		} else {
			// 縦線
			DrawLine(
				Vector3(-halfSize, 0.0f, z),
				Vector3(halfSize, 0.0f, z), color);
		}
		
	}
}

void GraphRenderer::SetCamera(Camera* camera) {
	assert(camera);
	camera_ = camera;
}

void GraphRenderer::DeleteCamera(Camera* camera) {
	if (camera_) {
		if (camera_ == camera) {
			camera_ = nullptr; // カメラを削除
		} 
	}
}
