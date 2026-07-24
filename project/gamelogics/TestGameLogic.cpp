#include "TestGameLogic.h"
#include "components/AllComponent.h"
#include <vector>

namespace {
	std::vector<QFE::SCRIPT::ScriptFunctionInfo> s_manifestList;
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
}