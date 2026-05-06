#pragma once
#include <functional>
#include "BridgeStructs.h"

namespace QFE {
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
		static std::function < void(const std::string&)> CreateCsharpScript;
		static std::function<std::vector<std::string>(uint32_t)> GetCsharpClassNames;
		static std::function<void(uint32_t, const std::string&)> RemoveCsharpScript;
		static std::function<void(uint32_t, const std::string&)> AddCsharpScript;
		static std::function<std::vector<std::string>()> GetAvailableCsharpClasses;
		static std::function<void()> ReCompileCsharpScripts;
		
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