#include "engine/include/scene/SceneCommand/ParentUpdateCommand.h"

#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "Engine/include/core/Math/ParentData.h"
#include "engine/include/core/Math/Transform.h"
#include "Engine/include/assets/3DModel/Data/ModelHandle.h"
#include "Engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "Engine/include/assets/Sprite/Data/SpriteData.h"

using namespace QFE;

ParentUpdateCommand::ParentUpdateCommand(EntityManager& entityManager) : ISceneEntityCommand(entityManager) {}

void ParentUpdateCommand::Execute()
{
	// 一時的に関数を削除
}

void ParentUpdateCommand::Undo()
{
	// 親子関係の更新は元に戻せない
}

std::string ParentUpdateCommand::GetCommandName() const
{
	return "Parent Update Command";
}
