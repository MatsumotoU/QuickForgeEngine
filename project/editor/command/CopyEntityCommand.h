#pragma once
#include "IEditorCommand.h"
#include <cstdint>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief エンティティを削除するコマンド
	class CopyEntityCommand : public IEditorCommand {
	public:
		CopyEntityCommand(uint32_t entityId, EntityManager* entityManager);

		/// @brief エンティティをコピーする。
		void Execute() override;
		/// @brief エンティティのコピーを消す。
		void Undo() override;

	private:
		EntityManager* entityManager_;
		uint32_t entityId_;
		uint32_t newEntityId_; // コピーされたエンティティのIDを保存する
	};
}
