#include "AttachScriptForEntity.h"
#include "Base/EngineCore.h"
#include "Object/Entity/EntityManager.h"
#include "Object/Component/Data/LuaScriptComponent.h"
#include "Object/System/Script/BaseSys/ScriptComponentCreater.h"

#include <cassert>

void AttachScriptForEntity::Attach(EngineCore* engineCore, EntityManager& entityManager, uint32_t entityId, const std::string& scriptName) {
	if (!engineCore) {
		assert(false && "EngineCore is null.");
		return;
	}

	LuaScriptComponent scriptComponent;
	ScriptComponentCreater::Create(engineCore, scriptComponent, scriptName);
	// スクリプト名が空でない場合のみアタッチ
	if (!scriptName.empty()) {
		entityManager.EmplaceComponent<LuaScriptComponent>(entityId, std::move(scriptComponent));
	}
}
