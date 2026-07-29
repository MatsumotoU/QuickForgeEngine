#include "PhysicsFramework.h"

bool QFE::FRAMEWORK::InitializePhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine) {
	engine.Initialize();
	return true;
}

bool QFE::FRAMEWORK::UpdatePhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine, float deltaTime) {
	engine.Update(deltaTime);
	return true;
}

bool QFE::FRAMEWORK::ShutdownPhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine) {
	engine.Shutdown();
	return true;
}
