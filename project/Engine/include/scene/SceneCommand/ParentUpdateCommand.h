#pragma once
#include "ISceneEntityCommand.h"

class ParentUpdateCommand final : public ISceneEntityCommand {
public:
	ParentUpdateCommand() = delete;
	explicit ParentUpdateCommand(EntityManager& entityManager);
	~ParentUpdateCommand() override = default;

	/// 親オブジェクトの変換情報を元に子オブジェクトのワールド行列を更新する
	void Execute() override;
	/// 親子関係の更新は元に戻せない
	void Undo() override;

	std::string GetCommandName() const override;
private:
	
};