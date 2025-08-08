#ifdef _DEBUG
#include "SceneEntityViewer.h"
#include "Object/Entity/EntityManager.h"
#include "Base/DirectX/ImGuiManager.h"

#include "Object/Component/Data/TransformComponent.h"
#include "Object/Component/Data/ParentComponent.h"
#include "Object/Component/Data/MaterialComponent.h"
#include "Object/Component/Data/MeshComponent.h"

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
			const auto& transform = entityManager.GetComponent<TransformComponent>(entityId);
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform.transform_.translate.x, transform.transform_.translate.y, transform.transform_.translate.z);
			ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform.transform_.rotate.x, transform.transform_.rotate.y, transform.transform_.rotate.z);
			ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform.transform_.scale.x, transform.transform_.scale.y, transform.transform_.scale.z);
		}
		if (entityManager.HasComponent<ParentComponent>(entityId)) {
			const auto& parent = entityManager.GetComponent<ParentComponent>(entityId);
			ImGui::Text("Parent Entity ID: %u", parent.parentEntityId);
		}
		if (entityManager.HasComponent<MaterialComponent>(entityId)) {
			const auto& material = entityManager.GetComponent<MaterialComponent>(entityId);
			ImGui::Text("Color: (%f,%f,%f,%f)",
				material.material_.GetData()->color.x,
				material.material_.GetData()->color.y,
				material.material_.GetData()->color.z,
				material.material_.GetData()->color.w);
		}
		if (entityManager.HasComponent<MeshComponent>(entityId)) {
			const auto& mesh = entityManager.GetComponent<MeshComponent>(entityId);
			ImGui::Text("Vertex Count: %u", mesh.vertices.size());
			ImGui::Text("Index Count: %u", mesh.indices.size());
		}
		ImGui::TreePop();
	}
}
#endif // _DEBUG