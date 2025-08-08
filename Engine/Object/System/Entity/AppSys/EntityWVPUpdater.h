#pragma once
#include "Object/System/Entity/BaseSys/MakeWVPForTransformComponent.h"
class EntityManager;
class Camera;

class EntityWVPUpdater {
public:
	static void Update(EntityManager& entityManager, Camera& camera);
};