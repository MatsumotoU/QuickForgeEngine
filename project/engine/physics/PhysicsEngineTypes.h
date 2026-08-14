#pragma once
#include <stdint.h>

namespace QFE::PHYSICS {
	/// @brief 物理エンジンにおける剛体のハンドルを表す列挙型
	enum class RigidBodyHandle : uint32_t { Invalid = UINT32_MAX };
	/// @brief 物理エンジンにおけるコライダーのハンドルを表す列挙型
	enum class ColliderHandle : uint32_t { Invalid = UINT32_MAX };
	/// @brief 物理エンジンにおけるコライダーのハンドルを表す列挙型
	enum class PhysicsMaterialHandle : uint32_t { Invalid = UINT32_MAX };
}
