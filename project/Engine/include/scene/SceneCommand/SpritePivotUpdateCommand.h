#pragma once
#include "ISceneEntityCommand.h"

class SpritePivotUpdateCommand final : public ISceneEntityCommand {
public:
	SpritePivotUpdateCommand() = delete;
	explicit SpritePivotUpdateCommand(EntityManager& entityManager);
	~SpritePivotUpdateCommand() override = default;

	/// 全スプライトのピボットを更新する
	void Execute() override;
	/// ピボットの更新は元に戻せない
	void Undo() override;

	std::string GetCommandName() const override;
};