#pragma once
#include "Object/Component/Data/TransformComponent.h"

class EngineCore;

class CreateTransformationComponent {
public:
	CreateTransformationComponent() = default;
	~CreateTransformationComponent() = default;
	// トランスフォームコンポーネントを作成
	static void Create(EngineCore* engineCore, TransformComponent& transformComponent);
};