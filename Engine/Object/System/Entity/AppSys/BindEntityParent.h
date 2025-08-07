#pragma once
#include "Object/Component/Data/ParentComponent.h"

class EntityManager;

class BindEntityParent {
public:
	BindEntityParent() = default;
	~BindEntityParent() = default;
	// エンティティの親クラスとしての機能を提供する
	void Bind(EntityManager& entityManager, uint32_t entityId, uint32_t parentEntityId);
};