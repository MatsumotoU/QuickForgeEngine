#include "EntityScriptUpdate.h"
#include "Base/EngineCore.h"
#include "Object/Component/Data/LuaScriptComponent.h"
#include "Object/Entity/EntityManager.h"

#include "Object/System/Script/BaseSys/RunningScript.h"

void EntityScriptUpdate::Update(EngineCore* engineCore, EntityManager& entityManager) {
	// EntityManagerからTransformComponentを取得し、WVP行列を更新
	if (!entityManager.HasComponentStrage<LuaScriptComponent>()) {
		return;
	}

	auto& luaStrage = entityManager.GetComponentStrage<LuaScriptComponent>();
	for (auto& [entityId, transform] : luaStrage) {
		LuaScriptComponent& luaScriptComponent = entityManager.GetComponent<LuaScriptComponent>(entityId);

		if (luaScriptComponent.scriptName.empty()) {
			continue; // スクリプト名が空の場合はスキップ
		}

		// LuaスクリプトのUpdate関数を実行
		for (const auto& funcName : luaScriptComponent.funcNames) {
			if (funcName == "Update") {// Update関数が存在する場合のみ実行
				RunningScript::Run(engineCore, &luaScriptComponent, "Update");
			}
		}
	}
}
