#pragma once
#include "BridgeStructInfo.h"

namespace QFE {
	/// @brief エンジンとエディタ間の橋渡し関数を定義するインターフェースクラス
	class IEngineBridge {
	public:
		IEngineBridge() = default;
		virtual ~IEngineBridge() = default;

		// コピーとムーブを禁止
		IEngineBridge(const IEngineBridge&) = delete;
		IEngineBridge& operator=(const IEngineBridge&) = delete;
		IEngineBridge(IEngineBridge&&) = delete;
		IEngineBridge& operator=(IEngineBridge&&) = delete;

		// ディレクトリ取得
		virtual std::string GetModelDirectoryPath() = 0;
		virtual std::string GetImageDirectoryPath() = 0;
		virtual std::string GetEntityTemplateDirectoryPath() = 0;
		virtual std::string GetScriptDirectoryPath() = 0;
		virtual std::string GetAnimationDirectoryPath() = 0;

		// Entity情報の取得・設定
		virtual std::vector<uint32_t> GetAllEntityIds() = 0;
		virtual std::string GetEntityName(uint32_t entityId) = 0;
		virtual void SetEntityName(uint32_t entityId, const std::string& name) = 0;
		virtual std::string GetEntityTag(uint32_t entityId) = 0;
		virtual void SetEntityTag(uint32_t entityId, const std::string& tag) = 0;

		// コンポーネント操作
		virtual bool HasComponent(uint32_t entityId, ComponentType type) = 0;
		virtual void AddComponent(uint32_t entityId, ComponentType type) = 0;
		virtual void RemoveComponent(uint32_t entityId, ComponentType type) = 0;

		// 特定のコンポーネントデータ
		virtual TransformData GetTransform(uint32_t entityId) = 0;
		virtual void SetTransform(uint32_t entityId, const TransformData& transform) = 0;

		virtual ModelRenderInfo GetModelRenderInfo(uint32_t entityId) = 0;
		virtual void SetMeshMaterial(uint32_t entityId, int meshIndex, const float color[4], float roughness) = 0;
		virtual void SetMeshLight(uint32_t entityId, int meshIndex, const float ambient[4], const float diffuse[3]) = 0;
		virtual void ChangeModel(uint32_t entityId, const std::string& modelPath) = 0;

		virtual ParticleInfo GetParticleInfo(uint32_t entityId) = 0;

		virtual SpriteInfo GetSpriteInfo(uint32_t entityId) = 0;
		virtual void SetSpriteInfo(uint32_t entityId, const SpriteInfo& spriteInfo) = 0;

		virtual CameraInfo GetCameraInfo(uint32_t entityId) = 0;
		virtual void SetCameraInfo(uint32_t entityId, const CameraInfo& cameraInfo) = 0;

		virtual ForceData GetForceData(uint32_t entityId) = 0;
		virtual void SetForceData(uint32_t entityId, const ForceData& forceData) = 0;

		virtual SphereColliderInfo GetSphereColliderInfo(uint32_t entityId) = 0;
		virtual void SetSphereColliderInfo(uint32_t entityId, const SphereColliderInfo& colliderInfo) = 0;

		virtual AABBColliderInfo GetAABBColliderInfo(uint32_t entityId) = 0;
		virtual void SetAABBColliderInfo(uint32_t entityId, const AABBColliderInfo& colliderInfo) = 0;

		virtual AnimationClipInfo GetAnimationInfo(uint32_t entityId) = 0;

		// スクリプト操作
		virtual void ReCompileCsharpScripts() = 0;
		virtual std::vector<std::string> GetCsharpClassNames(uint32_t entityId) = 0;
		virtual void RemoveCsharpScript(uint32_t entityId, const std::string& className) = 0;
		virtual void AddCsharpScript(uint32_t entityId, const std::string& className) = 0;
		virtual std::vector<std::string> GetAvailableCsharpClasses() = 0;

		// アニメーション結び付け
		virtual void PlayAnimation(uint32_t entityId,uint32_t animationHandle) = 0;
		virtual void BindAnimationClip(uint32_t entityId, const std::string& clipName) = 0;
		virtual std::vector<std::string> GetAvailableAnimationClips() = 0;

		// シーンにエンティティを追加する関数群
		virtual void AddEmptyEntity() = 0;
		virtual void AddSkyboxEntity(const std::string& filepath) = 0;
		virtual void AddEntityFromFile(const std::string& filepath) = 0;
		virtual void AddModelEntity(const std::string& filepath) = 0;
		virtual void AddSpriteEntity(const std::string& filepath) = 0;
		virtual void AddParticleEmitterEntity(const std::string& filepath, uint32_t particleCount) = 0;
		virtual void AddCameraEntity() = 0;

		// シーンにあるエンティティを操作する関数群
		virtual void CopyEntity(uint32_t entityId) = 0;
		virtual void SaveEntity(uint32_t entityId, std::string filename) = 0;
		virtual void DeleteEntity(uint32_t entityId) = 0;
		virtual void ParentChild(uint32_t parentEntityId, uint32_t childEntityId) = 0;
		virtual void Unparent(uint32_t entityId) = 0;

		// デバッグ用の関数群
		virtual uint32_t GetDebugCameraEntityId() = 0;
		virtual void ClearRuntimeDebugLogs() = 0;
	};
}