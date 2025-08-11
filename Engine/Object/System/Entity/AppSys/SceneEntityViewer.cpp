#ifdef _DEBUG
#include "SceneEntityViewer.h"
#include "Object/Entity/EntityManager.h"
#include "Base/DirectX/ImGuiManager.h"

#include "Object/Component/Data/TransformComponent.h"
#include "Object/Component/Data/ParentComponent.h"
#include "Object/Component/Data/MaterialComponent.h"
#include "Object/Component/Data/MeshComponent.h"
#include "Object/Component/Data/LuaScriptComponent.h"

void SceneEntityViewer::DisplayEntities(EntityManager& entityManager) {
	ImGui::Begin("Entity Viewer");
	for (uint32_t i = 0; i < entityManager.GetNextEntityId(); i++) {
		DisplayEntityDetails(entityManager, i);
	}
	ImGui::End();
}

void SceneEntityViewer::DisplayEntityDetails(EntityManager& entityManager, uint32_t entityId) {
	ImGui::Text("Entity ID: %u", entityId);
	if (ImGui::TreeNode(("Components##" + std::to_string(entityId)).c_str())) {
		if (entityManager.HasComponent<TransformComponent>(entityId)) {
			auto& transform = entityManager.GetComponent<TransformComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Transform Component:");
			ImGui::DragFloat3("Translate", &transform.transform_.translate.x, 0.1f);
			ImGui::DragFloat3("Rotate", &transform.transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.transform_.scale.x, 0.1f);
		}
		if (entityManager.HasComponent<ParentComponent>(entityId)) {
			const auto& parent = entityManager.GetComponent<ParentComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Parent Component:");
			ImGui::Text("Parent Entity ID: %u", parent.parentEntityId);
		}
		if (entityManager.HasComponent<MaterialComponent>(entityId)) {
			auto& material = entityManager.GetComponent<MaterialComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Material Component:");
			ImGui::DragFloat4("Color", &material.material_.GetData()->color.x, 0.01f);
		}
		if (entityManager.HasComponent<MeshComponent>(entityId)) {
			const auto& mesh = entityManager.GetComponent<MeshComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Mesh Component:");
			ImGui::Text("Vertex Count: %u", mesh.vertices.size());
			ImGui::Text("Index Count: %u", mesh.indices.size());
		}
		if (entityManager.HasComponent<ParentComponent>(entityId)) {
			const auto& parent = entityManager.GetComponent<ParentComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Parent Entity ID: %u", parent.parentEntityId);
		}
		if (entityManager.HasComponent<LuaScriptComponent>(entityId)) {
			const auto& luaScript = entityManager.GetComponent<LuaScriptComponent>(entityId);
			ImGui::Separator();
			ImGui::Text("Lua Script Component:");
			ImGui::Text("Script Name: %s", luaScript.scriptName.c_str());
			ImGui::Text("Functions:");
			for (const auto& funcName : luaScript.funcNames) {
				ImGui::BulletText("%s", funcName.c_str());
			}
			ImGui::Text("User Data:");
			for (const auto& valueName : luaScript.valueNames) {
				ImGui::BulletText("%s", valueName.c_str());
			}
		}
		ImGui::TreePop();
	}
}
#endif // _DEBUG