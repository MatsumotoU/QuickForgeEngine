#pragma once
#include <functional>
#include <string>
#include <cstdint>
#include <array>

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
	/// @brief エディタとエンジン間の橋渡しを行うクラス
	class EditorEngineBridge final {
	public:
		EditorEngineBridge() = default;
		~EditorEngineBridge() = default;
		// コピーとムーブを禁止
		EditorEngineBridge(const EditorEngineBridge&) = delete;
		EditorEngineBridge& operator=(const EditorEngineBridge&) = delete;
		EditorEngineBridge(EditorEngineBridge&&) = delete;
		EditorEngineBridge& operator=(EditorEngineBridge&&) = delete;

		// * エディタ側への機能の提供 * //
		// ディレクトリ取得
		static std::function<std::string()> GetModelDirectoryPath;
		static std::function<std::string()> GetImageDirectoryPath;
		static std::function<std::string()> GetEntityTemplateDirectoryPath;

		static std::function<std::vector<uint32_t>()> GetAllEntityIds;
		static std::function<std::string(uint32_t)> GetEntityName;
		static std::function<void(uint32_t, const std::string&)> SetEntityName;
		static std::function<std::string(uint32_t)> GetEntityTag;
		static std::function<void(uint32_t, const std::string&)> SetEntityTag;

		// コンポーネント操作
		static std::function<bool(uint32_t, ComponentType)> HasComponent;
		static std::function<void(uint32_t, ComponentType)> AddComponent;
		static std::function<void(uint32_t, ComponentType)> RemoveComponent;

		// 特定のコンポーネントデータ
		static std::function<TransformData(uint32_t)> GetTransform;
		static std::function<void(uint32_t, const TransformData&)> SetTransform;

		static std::function<ModelRenderInfo(uint32_t)> GetModelRenderInfo;
		static std::function<void(uint32_t, int, const float[4], float)> SetMeshMaterial;
		static std::function<void(uint32_t, int, const float[4], const float[3])> SetMeshLight;
		static std::function<void(uint32_t, const std::string&)> ChangeModel;

		static std::function<ParticleInfo(uint32_t)> GetParticleInfo;

		static std::function<SpriteInfo(uint32_t)> GetSpriteInfo;
		static std::function<void(uint32_t, const SpriteInfo&)> SetSpriteInfo;

		static std::function<CameraInfo(uint32_t)> GetCameraInfo;
		static std::function<void(uint32_t, const CameraInfo&)> SetCameraInfo;

		static std::function<ForceData(uint32_t)> GetForceData;
		static std::function<void(uint32_t, const ForceData&)> SetForceData;

		static std::function<SphereColliderInfo(uint32_t)> GetSphereColliderInfo;
		static std::function<void(uint32_t, const SphereColliderInfo&)> SetSphereColliderInfo;

		static std::function<AABBColliderInfo(uint32_t)> GetAABBColliderInfo;
		static std::function<void(uint32_t, const AABBColliderInfo&)> SetAABBColliderInfo;

		// スクリプト操作
		static std::function<std::vector<std::string>(uint32_t)> GetCsharpClassNames;
		static std::function<void(uint32_t, const std::string&)> RemoveCsharpScript;
		static std::function<void(uint32_t, const std::string&)> AddCsharpScript;
		static std::function<std::vector<std::string>()> GetAvailableCsharpClasses;
		
		// シーンにエンティティを追加する関数群
		static std::function<void()> AddEmptyEntity;
		static std::function<void(const std::string&)> AddEntityFromFile;
		static std::function<void(const std::string&)> AddModelEntity;
		static std::function<void(const std::string&)> AddSpriteEntity;
		static std::function<void(const std::string&, uint32_t)> AddParticleEmitterEntity;
		static std::function<void()> AddCameraEntity;
		// シーンにあるエンティティを操作する関数群
		static std::function<void(uint32_t)> CopyEntity;
		static std::function<void(uint32_t, std::string)> SaveEntity;
		static std::function<void(uint32_t)> DeleteEntity;
		static std::function<void(uint32_t, uint32_t)> ParentChild;
		static std::function<void(uint32_t)> Unparent;

		// デバッグ用の関数群
		static std::function<uint32_t()> GetDebugCameraEntityId;
	};
}