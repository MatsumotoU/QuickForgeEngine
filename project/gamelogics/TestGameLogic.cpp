#include "TestGameLogic.h"
#include "components/AllComponent.h"
#include <vector>
#include <cmath>

namespace {
	std::vector<QFE::SCRIPT::ScriptFunctionInfo> s_manifestList;

	struct FloatingComponent {
		bool enabled = true;
		float amplitude = 1.0f;
		float frequency = 1.0f;
		float elapsedTime = 0.0f;
	};

	void* CreateFloatingComponent() {
		return new FloatingComponent{};
	}

	void DestroyFloatingComponent(void* component) {
		delete static_cast<FloatingComponent*>(component);
	}

	void ReflectFloatingComponent(void* component, QFE::Archive& archive) {
		auto& floating = *static_cast<FloatingComponent*>(component);
		archive.Process("enabled", floating.enabled);
		archive.Process("amplitude", floating.amplitude);
		archive.Process("frequency", floating.frequency);
		// elapsedTimeはランタイム状態なので保存・編集しない
	}

	void UpdateFloatingComponent(
		uint32_t entityId, float deltaTime,
		QFE::IEntityManager* entityManager, void* component) {
		auto& floating = *static_cast<FloatingComponent*>(component);
		if (!floating.enabled) {
			return;
		}
		floating.elapsedTime += deltaTime;
		auto* transform = static_cast<QFE::SCENE::TransformComponent*>(
			entityManager->GetComponentRaw(entityId, "TransformComponent"));
		if (transform != nullptr) {
			transform->transform.translate.y +=
				std::sin(floating.elapsedTime * floating.frequency) *
				floating.amplitude * deltaTime;
		}
	}

	const QFE::DynamicComponentDescriptor s_componentManifest[] = {
		{
			1,
			"com.quickforge.sample.FloatingComponent",
			"Floating Component",
			CreateFloatingComponent,
			DestroyFloatingComponent,
			ReflectFloatingComponent,
			UpdateFloatingComponent
		}
	};
}

void MoveZ(uint32_t entityId, float dt, QFE::IEntityManager* entityManager) {
	auto* transform = 
		static_cast<QFE::SCENE::TransformComponent*>(entityManager->GetComponentRaw(entityId, "TransformComponent"));
	transform->transform.translate.z += 1.0f * dt;
}

extern "C" {
	
	__declspec(dllexport) size_t GetManifest(QFE::SCRIPT::ScriptFunctionInfo** outArray) {
		// 最初に1回だけリストを構築
		if (s_manifestList.empty()) {
			s_manifestList.push_back({ "MoveZ", "Move Z", MoveZ });
		}

		// 配列の先頭アドレスをEXE側に教えてあげる
		*outArray = s_manifestList.data();

		// 個数を返す
		return s_manifestList.size();
	}

	__declspec(dllexport) bool QFE_GetComponentManifest(
		QFE::PluginComponentManifest* outManifest) {
		if (outManifest == nullptr) {
			return false;
		}
		outManifest->apiVersion = 1;
		outManifest->components = s_componentManifest;
		outManifest->componentCount = std::size(s_componentManifest);
		return true;
	}
}
