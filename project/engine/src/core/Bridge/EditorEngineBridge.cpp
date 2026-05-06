#include "engine/include/core/Bridge/EditorEngineBridge.h"
#ifdef  QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif //  QFE_OPTIMIZE_OFF

namespace QFE {
	// * エディタ側への機能の提供 * //
	// ディレクトリ取得
	std::function<std::string()> EditorEngineBridge::GetModelDirectoryPath = nullptr;
	std::function<std::string()> EditorEngineBridge::GetImageDirectoryPath = nullptr;
	std::function<std::string()> EditorEngineBridge::GetEntityTemplateDirectoryPath = nullptr;

	// 現在のシーン情報を取得する関数群
	std::function<std::vector<uint32_t>()> EditorEngineBridge::GetAllEntityIds = nullptr;
	std::function<std::string(uint32_t)> EditorEngineBridge::GetEntityName = nullptr;
	std::function<void(uint32_t, const std::string&)> EditorEngineBridge::SetEntityName = nullptr;
	std::function<std::string(uint32_t)> EditorEngineBridge::GetEntityTag = nullptr;
	std::function<void(uint32_t, const std::string&)> EditorEngineBridge::SetEntityTag = nullptr;

	// コンポーネント操作
	std::function<bool(uint32_t, ComponentType)> EditorEngineBridge::HasComponent = nullptr;
	std::function<void(uint32_t, ComponentType)> EditorEngineBridge::AddComponent = nullptr;
	std::function<void(uint32_t, ComponentType)> EditorEngineBridge::RemoveComponent = nullptr;

	// 特定のコンポーネントデータ
	std::function<TransformData(uint32_t)> EditorEngineBridge::GetTransform = nullptr;
	std::function<void(uint32_t, const TransformData&)> EditorEngineBridge::SetTransform = nullptr;

	std::function<ModelRenderInfo(uint32_t)> EditorEngineBridge::GetModelRenderInfo = nullptr;
	std::function<void(uint32_t, int, const float[4], float)> EditorEngineBridge::SetMeshMaterial = nullptr;
	std::function<void(uint32_t, int, const float[4], const float[3])> EditorEngineBridge::SetMeshLight = nullptr;
	std::function<void(uint32_t, const std::string&)> EditorEngineBridge::ChangeModel = nullptr;

	std::function<ParticleInfo(uint32_t)> EditorEngineBridge::GetParticleInfo = nullptr;

	std::function<SpriteInfo(uint32_t)> EditorEngineBridge::GetSpriteInfo = nullptr;
	std::function<void(uint32_t, const SpriteInfo&)> EditorEngineBridge::SetSpriteInfo = nullptr;

	std::function<CameraInfo(uint32_t)> EditorEngineBridge::GetCameraInfo = nullptr;
	std::function<void(uint32_t, const CameraInfo&)> EditorEngineBridge::SetCameraInfo = nullptr;

	std::function<ForceData(uint32_t)> EditorEngineBridge::GetForceData = nullptr;
	std::function<void(uint32_t, const ForceData&)> EditorEngineBridge::SetForceData = nullptr;

	std::function<SphereColliderInfo(uint32_t)> EditorEngineBridge::GetSphereColliderInfo = nullptr;
	std::function<void(uint32_t, const SphereColliderInfo&)> EditorEngineBridge::SetSphereColliderInfo = nullptr;

	std::function<AABBColliderInfo(uint32_t)> EditorEngineBridge::GetAABBColliderInfo = nullptr;
	std::function<void(uint32_t, const AABBColliderInfo&)> EditorEngineBridge::SetAABBColliderInfo = nullptr;

	// スクリプト操作
	std::function<void(const std::string&)> EditorEngineBridge::CreateCsharpScript = nullptr;
	std::function<std::vector<std::string>(uint32_t)> EditorEngineBridge::GetCsharpClassNames = nullptr;
	std::function<void(uint32_t, const std::string&)> EditorEngineBridge::RemoveCsharpScript = nullptr;
	std::function<void(uint32_t, const std::string&)> EditorEngineBridge::AddCsharpScript = nullptr;
	std::function<std::vector<std::string>()> EditorEngineBridge::GetAvailableCsharpClasses = nullptr;
	std::function<void()> EditorEngineBridge::ReCompileCsharpScripts = nullptr;

	// シーンにエンティティを追加する関数群
	std::function<void(void)> EditorEngineBridge::AddEmptyEntity = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddEntityFromFile = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddModelEntity = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddSpriteEntity = nullptr;
	std::function<void(const std::string&, uint32_t)> EditorEngineBridge::AddParticleEmitterEntity = nullptr;
	std::function<void(void)> EditorEngineBridge::AddCameraEntity = nullptr;
	std::function<void(uint32_t)> EditorEngineBridge::CopyEntity = nullptr;
	std::function<void(uint32_t, std::string)> EditorEngineBridge::SaveEntity = nullptr;
	std::function<void(uint32_t)> EditorEngineBridge::DeleteEntity = nullptr;
	std::function<void(uint32_t, uint32_t)> EditorEngineBridge::ParentChild = nullptr;
	std::function<void(uint32_t)> EditorEngineBridge::Unparent = nullptr;
	// デバッグ用の関数群
	std::function<uint32_t()> EditorEngineBridge::GetDebugCameraEntityId = nullptr;
}