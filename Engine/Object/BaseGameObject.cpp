#include "BaseGameObject.h"
#ifdef _DEBUG
#include "Camera/Camera.h"
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG


void BaseGameObject::DrawGizmo(const ImGuizmo::OPERATION& op, const ImVec2& imageScreenPos, const ImVec2& imageSize) {// ギズモ表示
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
		ImGuizmo::MODE currentGizmoMode = ImGuizmo::WORLD;

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
}
