#pragma once
#include "ISceneEntityCommand.h"
#include <nlohmann/json.hpp>
namespace QFE {
	class DeleteSceneEntityCommand final : public ISceneEntityCommand {
	public:
		DeleteSceneEntityCommand() = delete;
		/// 必ずシーン始めにコマンドを生成すること
		explicit DeleteSceneEntityCommand(EntityManager& entityManager, uint32_t entityId);
		~DeleteSceneEntityCommand() override = default;

		/// 指定されたエンティティをシーンから削除する
		void Execute() override;
		/// 削除されたエンティティをシーンに復元する
		void Undo() override;

		std::string GetCommandName() const override;
	private:
		uint32_t entityId_;
		nlohmann::json serializedEntityJson_;
	};
}