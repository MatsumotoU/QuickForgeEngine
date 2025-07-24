#include "BaseGameObject.h"

BaseGameObject::BaseGameObject(EngineCore* engineCore) {
    localPos_ = {};
    worldPos_ = {};
    worldMatrix_ = Matrix4x4::MakeIndentity4x4();

	transform_ = Transform();
	engineCore_ = engineCore;
}

Vector3 BaseGameObject::GetWorldPos() {
    return Vector3::Transform(localPos_,worldMatrix_);
}