#pragma once
#include "Object/Component/Data/TransformComponent.h"
class Camera;

class MakeWVPForTransformComponent {
public:
	static void Make(TransformComponent& transform,Camera& camera);
};