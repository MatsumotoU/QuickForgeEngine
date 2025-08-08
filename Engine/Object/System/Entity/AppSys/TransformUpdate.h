#pragma once
#include "Object/Component/Data/TransformComponent.h"
#include "Object/Entity/EntityManager.h"

class Camera;

class TransformUpdate {
public:
	TransformUpdate() = default;
	~TransformUpdate() = default;
	// エンティティのトランスフォームを更新する
	static void Update(EntityManager& entityManager,Camera& camera);
};