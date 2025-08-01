#include "BaseGameObject.h"
#ifdef _DEBUG
#include "Camera/Camera.h"
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG

#ifdef _DEBUG
void BaseGameObject::DrawImGui() {
	// 名前の表示
	ImGui::Text("Name: %s", name_.c_str());
	// タグの表示
	ImGui::Text("Tag: %s",tag_.c_str());
	// スクリプト名の表示
	ImGui::Text("Script Name: %s", attachedScriptName.c_str());
	// Transformの表示
	ImGui::Text("Transform");
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);

	// ワールド行列の表示
	if (ImGui::TreeNode("WorldMatrix")) {
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[0][0], worldMatrix_.m[0][1], worldMatrix_.m[0][2], worldMatrix_.m[0][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[1][0], worldMatrix_.m[1][1], worldMatrix_.m[1][2], worldMatrix_.m[1][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[2][0], worldMatrix_.m[2][1], worldMatrix_.m[2][2], worldMatrix_.m[2][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2], worldMatrix_.m[3][3]);
		ImGui::TreePop();
	}

	// コライダーの表示
	if (ImGui::TreeNode("Collider")) {
		float radius = GetRadius();
		if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f)) {
			SetRadius(radius);
		}
		ImGui::Text("Mask: %u", GetMask());
		ImGui::Text("IsHitOther %s", IsHit() ? "Ture" : "False");
		ImGui::TreePop();
	}
}

void BaseGameObject::DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize) {// ギズモ表示
	if (camera_) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		// ImGui::Imageの表示位置・サイズに合わせる
		ImGuizmo::SetRect(imageScreenPos.x, imageScreenPos.y, imageSize.x, imageSize.y);

		// 行列をfloat配列に変換
		Matrix4x4 world = worldMatrix_;
		float worldMatrix[16];
		std::memcpy(worldMatrix, &world, sizeof(float) * 16);

		Matrix4x4 view = camera_->viewMatrix_;
		float viewMatrix[16];
		std::memcpy(viewMatrix, &view, sizeof(float) * 16);

		Matrix4x4 proj = camera_->perspectiveMatrix_;
		float projMatrix[16];
		std::memcpy(projMatrix, &proj, sizeof(float) * 16);

		ImGuizmo::OPERATION currentGizmoOperation = op;
		ImGuizmo::MODE currentGizmoMode = mode;

		ImGuizmo::Manipulate(viewMatrix, projMatrix, currentGizmoOperation, currentGizmoMode, worldMatrix);

		// ギズモで編集された場合、transform_に反映
		if (ImGuizmo::IsUsing()) {
			Vector3 scale, rotation, translation;
			this->DecomposeMatrix(worldMatrix, scale, rotation, translation);
			transform_.scale = scale;
			transform_.rotate = rotation;
			transform_.translate = translation;
		}
	}
}

void BaseGameObject::DecomposeMatrix(const float* matrix, Vector3& scale, Vector3& rotation, Vector3& translation) {
	float s[3], r[3], t[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix, t, r, s);
	translation = { t[0], t[1], t[2] };
	// 回転は度→ラジアンに変換
	constexpr float DegToRad = 3.14159265358979323846f / 180.0f;
	rotation = { r[0] * DegToRad, r[1] * DegToRad, r[2] * DegToRad };
	scale = { s[0], s[1], s[2] };
}
#endif // _DEBUG
Vector3 BaseGameObject::GetWorldPosition() {
	return Vector3::Transform(localPos_, worldMatrix_);
}

BaseGameObject::BaseGameObject(EngineCore* engineCore, Camera* camera) {
    localPos_ = {};
    worldPos_ = {};
    worldMatrix_ = Matrix4x4::MakeIndentity4x4();

    transform_ = Transform();
    engineCore_ = engineCore;
	camera_ = camera;

	timeCounter_ = 0.0f;
}
