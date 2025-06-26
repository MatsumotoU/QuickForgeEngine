#include "BaseGameObject.h"

BaseGameObject::BaseGameObject() {
    localPos_ = {};
    worldPos_ = {};
    worldMatrix_ = Matrix4x4::MakeIndentity4x4();
}

Vector3 BaseGameObject::GetWorldPos() {
    return Vector3::Transform(localPos_,worldMatrix_);
}