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
	/// @brief コンポーネントをエンティティに追加するコマンドクラス
	class AddComponentCommand : public IEditorCommand {
	public:
		AddComponentCommand(uint32_t entityId, EntityManager* entityManager, const std::string& componentName);

		/// @brief エンティティをコピーする。
		void Execute() override;
		/// @brief エンティティのコピーを消す。
		void Undo() override;

	private:
		EntityManager* entityManager_;
		uint32_t entityId_;
		std::string addComponentName_;
	};
}
