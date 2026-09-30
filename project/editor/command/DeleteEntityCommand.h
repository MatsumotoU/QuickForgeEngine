#pragma once
#include "IEditorCommand.h"
#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief エンティティを削除するコマンド
	class DeleteEntityCommand : public IEditorCommand {
	public:
		DeleteEntityCommand(uint32_t entityId, EntityManager* entityManager);

		/// @brief エンティティを削除する。
		void Execute() override;
		/// @brief エンティティを復元する。
		void Undo() override;

	private:
		EntityManager* entityManager_;
		uint32_t entityId_;
		std::vector<std::pair<uint32_t, nlohmann::json>> removedEntities_;
	};
}
