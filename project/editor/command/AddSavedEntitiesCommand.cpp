#include "AddSavedEntitiesCommand.h"
#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"
#include <unordered_set>
#include <utility>

QFE::EDITOR::AddSavedEntitiesCommand::AddSavedEntitiesCommand(SCENE::SceneManager* scene, std::string path, QFE::MATH::Vector3 origin)
	: scene_(scene), path_(std::move(path)), origin_(origin) {}

void QFE::EDITOR::AddSavedEntitiesCommand::Execute() {
	auto& manager = QFE::FRAMEWORK::GetSceneEntities(*scene_);
	const auto before = manager.GetActiveEntityIds();
	const std::unordered_set<uint32_t> oldIds(before.begin(), before.end());
	created_.clear();
	if (QFE::FRAMEWORK::AppendScene(*scene_, path_, origin_) == UINT32_MAX) return;
	// 複数Entityを含むファイルも、追加されたIDをまとめてUndoできるようにする。
	for (const auto id : manager.GetActiveEntityIds()) {
		if (!oldIds.contains(id)) created_.push_back(id);
	}
}

void QFE::EDITOR::AddSavedEntitiesCommand::Undo() {
	auto& manager = QFE::FRAMEWORK::GetSceneEntities(*scene_);
	for (const auto id : created_) manager.RemoveEntity(id);
	created_.clear();
}
