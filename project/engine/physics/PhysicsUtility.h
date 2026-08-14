#pragma once
#include "math/MathInclude.h"
#include "data/Force.h"
#include "data/PhysicsMaterial.h"

namespace QFE::PHYSICS {
	/// @brief 物理エンジンにおける力の更新を行う関数
	bool UpdateForce(Force* force, PhysicsMaterial* material, float deltaTime);
}