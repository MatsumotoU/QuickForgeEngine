#pragma once
#include "IEngineBridge.h"
#include "engine/include/WindowsEngineCore.h"

namespace QFE {

	class WindowsBridgeCore final : public IEngineBridge {
	public:
		explicit WindowsBridgeCore(WindowsEngineCore* engineCore) : engineCore_(engineCore) {}
		virtual ~WindowsBridgeCore() = default;
		// コピーとムーブを禁止
		WindowsBridgeCore(const WindowsBridgeCore&) = delete;
		WindowsBridgeCore& operator=(const WindowsBridgeCore&) = delete;
		WindowsBridgeCore(WindowsBridgeCore&&) = delete;
		WindowsBridgeCore& operator=(WindowsBridgeCore&&) = delete;

		// ディレクトリ取得
		virtual std::string GetModelDirectoryPath() override;
		virtual std::string GetImageDirectoryPath() override;
		virtual std::string GetEntityTemplateDirectoryPath() override;
		virtual std::string GetScriptDirectoryPath() override;

		// Entity情報の取得・設定
		virtual std::vector<uint32_t> GetAllEntityIds() override;
		virtual std::string GetEntityName(uint32_t entityId) override;
		virtual void SetEntityName(uint32_t entityId, const std::string& name) override;
		virtual std::string GetEntityTag(uint32_t entityId) override;
		virtual void SetEntityTag(uint32_t entityId, const std::string& tag) override;

		// コンポーネント操作
		virtual bool HasComponent(uint32_t entityId, ComponentType type) override;
		virtual void AddComponent(uint32_t entityId, ComponentType type) override;
		virtual void RemoveComponent(uint32_t entityId, ComponentType type) override;

		// 特定のコンポーネントデータ
		virtual TransformData GetTransform(uint32_t entityId) override;
		virtual void SetTransform(uint32_t entityId, const TransformData& transform) override;

		virtual ModelRenderInfo GetModelRenderInfo(uint32_t entityId) override;
		virtual void SetMeshMaterial(uint32_t entityId, int meshIndex, const float color[4], float roughness) override;
		virtual void SetMeshLight(uint32_t entityId, int meshIndex, const float ambient[4], const float diffuse[3]) override;
		virtual void ChangeModel(uint32_t entityId, const std::string& modelPath) override;

		virtual ParticleInfo GetParticleInfo(uint32_t entityId) override;

		virtual SpriteInfo GetSpriteInfo(uint32_t entityId) override;
		virtual void SetSpriteInfo(uint32_t entityId, const SpriteInfo& spriteInfo) override;

		virtual CameraInfo GetCameraInfo(uint32_t entityId) override;
		virtual void SetCameraInfo(uint32_t entityId, const CameraInfo& cameraInfo) override;

		virtual ForceData GetForceData(uint32_t entityId) override;
		virtual void SetForceData(uint32_t entityId, const ForceData& forceData) override;

		virtual SphereColliderInfo GetSphereColliderInfo(uint32_t entityId) override;
		virtual void SetSphereColliderInfo(uint32_t entityId, const SphereColliderInfo& colliderInfo) override;

		virtual AABBColliderInfo GetAABBColliderInfo(uint32_t entityId) override;
		virtual void SetAABBColliderInfo(uint32_t entityId, const AABBColliderInfo& colliderInfo) override;

		// スクリプト操作
		virtual std::vector<std::string> GetCsharpClassNames(uint32_t entityId) override;
		virtual void RemoveCsharpScript(uint32_t entityId, const std::string& className) override;
		virtual void AddCsharpScript(uint32_t entityId, const std::string& className) override;
		virtual std::vector<std::string> GetAvailableCsharpClasses() override;

		// シーンにエンティティを追加する関数群
		virtual void ReCompileCsharpScripts() override;
		virtual void AddEmptyEntity() override;
		virtual void AddEntityFromFile(const std::string& filepath) override;
		virtual void AddModelEntity(const std::string& filepath) override;
		virtual void AddSpriteEntity(const std::string& filepath) override;
		virtual void AddParticleEmitterEntity(const std::string& filepath, uint32_t particleCount) override;
		virtual void AddCameraEntity() override;

		// シーンにあるエンティティを操作する関数群
		virtual void CopyEntity(uint32_t entityId) override;
		virtual void SaveEntity(uint32_t entityId, std::string filename) override;
		virtual void DeleteEntity(uint32_t entityId) override;
		virtual void ParentChild(uint32_t parentEntityId, uint32_t childEntityId) override;
		virtual void Unparent(uint32_t entityId) override;

		// デバッグ用の関数群
		virtual uint32_t GetDebugCameraEntityId() override;
		virtual void ClearRuntimeDebugLogs() override;
	private:
		WindowsEngineCore* engineCore_ = nullptr;
	};
}