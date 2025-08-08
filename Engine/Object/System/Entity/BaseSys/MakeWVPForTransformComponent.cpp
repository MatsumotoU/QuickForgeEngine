#include "MakeWVPForTransformComponent.h"
#include "Object/Component/Data/TransformComponent.h"
#include "Camera/Camera.h"

void MakeWVPForTransformComponent::Make(TransformComponent& transform, Camera& camera) {
	// WVP行列を計算
	transform.transformationBuffer_.GetData()->WVP =
		camera.MakeWorldViewProjectionMatrix(transform.transformationBuffer_.GetData()->World, CAMERA_VIEW_STATE_PERSPECTIVE);
}