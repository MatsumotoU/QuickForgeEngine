#pragma once
#include "ISceneEntityCommand.h"

namespace QFE {
	class ListUniqueIDManager;

	class RemakeUniqeIDCommand final : public ISceneEntityCommand {
	public:
		RemakeUniqeIDCommand() = delete;
		explicit RemakeUniqeIDCommand(EntityManager& entityManager, ListUniqueIDManager& uidManager);
		~RemakeUniqeIDCommand() override = default;

		/// 全オブジェクトのユニークIDを再生成する
		void Execute() override;
		/// ユニークIDの再生成は元に戻せない
		void Undo() override;

		std::string GetCommandName() const override;

	private:
		ListUniqueIDManager& uniqueIdManager_;
	};
}