#pragma once
#include "physics/PhysicsEngine.h"

namespace QFE::FRAMEWORK {
	/// @brief 物理エンジンの初期化を行う関数
	bool InitializePhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine);
	/// @brief 物理エンジンの更新を行う関数
	bool UpdatePhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine, float deltaTime);
	/// @brief 物理エンジンのシャットダウンを行う関数
	bool ShutdownPhysicsFramework(QFE::PHYSICS::PhysicsEngine& engine);
}