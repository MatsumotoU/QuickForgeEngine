#pragma once
class EntityManager;
#include "ISceneEntityCommand.h"

class WorldTransformationCommand : public ISceneEntityCommand {
public:
	WorldTransformationCommand() = delete;
	WorldTransformationCommand(EntityManager& em);

	/// モデル、スプライト、パーティクルのワールド行列を更新する
	void Execute() override;
	/// ワールド行列の更新は元に戻せない
	void Undo() override;

	std::string GetCommandName() const override;
private:

};