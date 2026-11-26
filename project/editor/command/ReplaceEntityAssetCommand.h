#pragma once

#include "IEditorCommand.h"
#include "components/ObjectInfoComponent.h"
#include "components/ParentComponent.h"
#include "components/TransformComponent.h"
#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"
#include <algorithm>
#include <functional>
#include <cmath>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

namespace QFE::EDITOR {
	// 元の Entity ID と UUID を残すことで、外部からの参照や既存の子を維持する。
	class ReplaceEntityAssetCommand final : public IEditorCommand {
	public:
		ReplaceEntityAssetCommand(SCENE::SceneManager* scene, std::string path, std::set<uint32_t> targets)
			: scene_(scene), path_(std::move(path)), targets_(std::move(targets)) {}
		void Execute() override {
			auto& entities = FRAMEWORK::GetSceneEntities(*scene_);
			if (initialized_) {
				for (const auto& [id, data] : added_) {
					(void)data;
					entities.CancelEntityRemoval(id);
					if (!entities.IsActiveEntity(id)) entities.ForceCreateEntity(id);
				}
				for (const auto& [id, data] : after_) Apply(entities, id, data);
				for (const auto& [id, data] : added_) Apply(entities, id, data);
				return;
			}
			for (uint32_t id : targets_) {
				if (!entities.IsActiveEntity(id) || !entities.HasComponent<SCENE::ObjectInfoComponent>(id)) continue;
				before_.emplace_back(id, entities.SerializeEntityComponents(id));
			}
			for (const auto& [target, original] : before_) {
				const auto previous = entities.GetActiveEntityIds();
				const std::unordered_set<uint32_t> previousIds(previous.begin(), previous.end());
				bool replaced = false;
				try {
					replaced = ReplaceOne(entities, target, original);
				} catch (const std::exception& error) {
					QFE_LOG(std::string("Failed to read replacement entity: ") + error.what(), QFE::LogLevel::Error);
				}
				if (!replaced) {
					// 途中で失敗しても、先に置換した対象を含めて元へ戻す。
					for (uint32_t id : entities.GetActiveEntityIds())
						if (!previousIds.contains(id) && entities.IsActiveEntity(id)) entities.InstantRemoveEntity(id);
					for (const auto& [id, data] : added_) { (void)data; if (entities.IsActiveEntity(id)) entities.InstantRemoveEntity(id); }
					for (const auto& [id, data] : before_) Apply(entities, id, data);
					added_.clear(); after_.clear(); before_.clear();
					QFE_LOG("Failed to replace entities with the selected asset.", QFE::LogLevel::Error);
					initialized_ = true;
					return;
				}
			}
			for (const auto& [id, data] : before_) { (void)data; after_.emplace_back(id, entities.SerializeEntityComponents(id)); }
			initialized_ = true;
		}
		void Undo() override {
			auto& entities = FRAMEWORK::GetSceneEntities(*scene_);
			for (const auto& [id, data] : added_) { (void)data; if (entities.IsActiveEntity(id)) entities.RemoveEntity(id); }
			for (const auto& [id, data] : before_) Apply(entities, id, data);
		}
	private:
		using Snapshot = std::vector<std::pair<uint32_t, nlohmann::json>>;
		static void Apply(EntityManager& entities, uint32_t id, const nlohmann::json& data) {
			for (const auto& type : entities.GetComponentTypeNames(id)) entities.RemoveComponent(id, type.c_str());
			entities.DeserializeEntityComponents(id, data);
		}
		bool ReplaceOne(EntityManager& entities, uint32_t target, const nlohmann::json& original) {
			const auto previous = entities.GetActiveEntityIds();
			const std::unordered_set<uint32_t> oldIds(previous.begin(), previous.end());
			if (FRAMEWORK::AppendScene(*scene_, path_, {}) == UINT32_MAX) return false;
			std::vector<uint32_t> created;
			for (uint32_t id : entities.GetActiveEntityIds()) if (!oldIds.contains(id)) created.push_back(id);
			std::sort(created.begin(), created.end());
			if (created.empty()) return false;
			std::unordered_set<std::string> internalUuids;
			for (uint32_t id : created) {
				const auto* info = entities.GetComponentPtr<SCENE::ObjectInfoComponent>(id);
				if (info && !info->uuid.empty()) internalUuids.insert(info->uuid);
			}
			uint32_t root = UINT32_MAX;
			for (uint32_t id : created) {
				const auto* parent = entities.GetComponentPtr<SCENE::ParentComponent>(id);
				if (!parent || !internalUuids.contains(parent->parent.uuid)) { root = id; break; }
			}
			if (root == UINT32_MAX) {
				for (uint32_t id : created) if (entities.IsActiveEntity(id)) entities.InstantRemoveEntity(id);
				return false;
			}
			auto* rootInfo = entities.GetComponentPtr<SCENE::ObjectInfoComponent>(root);
			if (!rootInfo) rootInfo = &entities.AddDefaultComponent<SCENE::ObjectInfoComponent>(root);
			const std::string rootUuid = rootInfo->uuid;
			const auto& targetInfo = entities.GetComponent<SCENE::ObjectInfoComponent>(target);
			const std::string targetUuid = targetInfo.uuid.empty() ? QFE::GenerateEntityUuid() : targetInfo.uuid;
			const std::string targetGroup = targetInfo.hierarchyGroup;
			const auto* rootTransform = entities.GetComponentPtr<SCENE::TransformComponent>(root);
			if (rootTransform && (std::abs(rootTransform->transform.scale.x) < 1.0e-6f ||
				std::abs(rootTransform->transform.scale.y) < 1.0e-6f || std::abs(rootTransform->transform.scale.z) < 1.0e-6f)) {
				// 逆行列が作れないルートは相対配置を壊すため、追加分を消して中止する。
				for (uint32_t id : created) if (entities.IsActiveEntity(id)) entities.InstantRemoveEntity(id);
				return false;
			}
			const auto rootMatrix = rootTransform ? MATH::Matrix4x4::MakeAffineMatrix(rootTransform->transform) : MATH::Matrix4x4::MakeIdentity4x4();
			const auto inverseRoot = rootMatrix.Inverse();
			const auto remap = [&](nlohmann::json& data) {
				const std::function<void(nlohmann::json&)> visit = [&](nlohmann::json& value) {
					if (value.is_string() && value.get<std::string>() == rootUuid) value = targetUuid;
					else if (value.is_array() || value.is_object()) for (auto& child : value) visit(child);
				};
				if (!rootUuid.empty()) visit(data);
			};
			auto rootData = entities.SerializeEntityComponents(root);
			remap(rootData);
			rootData["ObjectInfoComponent"]["uuid"] = targetUuid;
			rootData["ObjectInfoComponent"]["hierarchyGroup"] = targetGroup;
			if (original.contains("TransformComponent")) rootData["TransformComponent"] = original["TransformComponent"];
			// 元に Transform がない場合は、アセットの初期 Transform を使う。
			if (original.contains("ParentComponent")) rootData["ParentComponent"] = original["ParentComponent"];
			else rootData.erase("ParentComponent");
			for (uint32_t id : created) {
				if (id == root) continue;
				const auto* parent = entities.GetComponentPtr<SCENE::ParentComponent>(id);
				const bool separateRoot = !parent || !internalUuids.contains(parent->parent.uuid);
				auto data = entities.SerializeEntityComponents(id);
				remap(data);
				if (data.contains("ObjectInfoComponent")) data["ObjectInfoComponent"]["hierarchyGroup"] = targetGroup;
				if (separateRoot) {
					// 複数のルートは代表ルートからの相対位置を保って選択対象の子にする。
					const auto* transform = entities.GetComponentPtr<SCENE::TransformComponent>(id);
					const auto matrix = transform ? MATH::Matrix4x4::MakeAffineMatrix(transform->transform) : MATH::Matrix4x4::MakeIdentity4x4();
					SCENE::TransformComponent relative{};
					relative.transform.FromMatrix(MATH::Matrix4x4::Multiply(matrix, inverseRoot));
					nlohmann::json value;
					JsonArchive archive(value, false);
					relative.Reflect(archive);
					data["TransformComponent"] = std::move(value);
					data["ParentComponent"] = { { "parent", targetUuid } };
				}
				Apply(entities, id, data);
				added_.emplace_back(id, entities.SerializeEntityComponents(id));
			}
			// 子の親参照を付け替えた後に一時ルートだけを消す。
			entities.InstantRemoveEntity(root);
			Apply(entities, target, rootData);
			return true;
		}
		SCENE::SceneManager* scene_;
		std::string path_;
		std::set<uint32_t> targets_;
		Snapshot before_, after_, added_;
		bool initialized_ = false;
	};
}
