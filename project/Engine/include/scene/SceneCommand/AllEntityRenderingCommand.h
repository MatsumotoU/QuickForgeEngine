#pragma once
#include "ISceneEntityCommand.h"
namespace QFE {
	class AllEntityRenderingCommand final : public ISceneEntityCommand {
	public:
		AllEntityRenderingCommand() = delete;
		explicit AllEntityRenderingCommand(EntityManager& entityManager);
		~AllEntityRenderingCommand() override = default;

		/// シーン内の全エンティティのレンダリングを実行する
		void Execute() override;
		/// レンダリングコマンドは元に戻せない
		void Undo() override;

		std::string GetCommandName() const override;
	private:
	};
}