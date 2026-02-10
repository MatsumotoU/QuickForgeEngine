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
	// デバッグ用の関数群
	std::function<uint32_t()> EditorEngineBridge::GetDebugCameraEntityId = nullptr;
}