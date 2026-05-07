#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace QFE {
	/// @brief コンポーネントの種類
	enum class ComponentType {
		Transform,
		SceneObjectData,
		ModelHandle,
		SpriteData,
		CameraData,
		Billboard,
		Particle,
		PhysicsForce,
		SphereCollider,
		AABBCollider,
		CsharpScript,
		ParentData
	};

	/// @brief Transformのデータ構造体
	struct TransformData {
		float translate[3];
		float rotate[3];
		float scale[3];
	};

	/// @brief メッシュの情報
	struct MeshInfo {
		float materialColor[4];
		float materialShininess;
		float lightColor[4];
		float lightDirection[3];
	};

	/// @brief モデルの描画情報
	struct ModelRenderInfo {
		std::string modelName;
		std::vector<MeshInfo> meshes;
	};

	/// @brief パーティクルコンポーネントの情報
	struct ParticleInfo {
		std::string modelName;
		uint32_t maxParticleCount;
	};

	/// @brief スプライトコンポーネントの情報
	struct SpriteInfo {
		std::string fileName;
		bool isBillboard;
		float width;
		float height;
		float pivot[2];
	};

	/// @brief カメラコンポーネントの情報
	struct CameraInfo {
		float fov;
		float nearZ;
		float farZ;
	};

	/// @brief 物理（Force）コンポーネントの情報
	struct ForceData {
		float velocity[3];
		float acceleration[3];
		float mass;
		float friction;
		float gravityStrength;
		bool isGravity;
	};

	/// @brief 球体コライダーの情報
	struct SphereColliderInfo {
		float center[3];
		float radius;
		bool isTrigger;
		bool isStatic;
		uint8_t colliderLayer;
		uint8_t eventColliderLayer;
		bool isDraw;
	};

	/// @brief AABBコライダーの情報
	struct AABBColliderInfo {
		float center[3];
		float size[3];
		bool isTrigger;
		bool isStatic;
		uint8_t colliderLayer;
		uint8_t eventColliderLayer;
		bool isDraw;
	};

	/// @brief スクリプトパラメータの型
	enum class ScriptParamType {
		Int,
		Float,
		Bool,
		String,
		Unknown
	};

	/// @brief スクリプトパラメータの情報
	struct ScriptParamInfo {
		std::string name;
		ScriptParamType type;
		std::string value;
	};

	/// @brief アタッチされているスクリプトの情報
	struct ScriptInfo {
		std::string name;
		uint32_t handle;
		int priority;
		std::vector<ScriptParamInfo> params;
	};
}