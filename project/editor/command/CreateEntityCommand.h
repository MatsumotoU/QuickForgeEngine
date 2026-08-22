#pragma once
#include "IEditorCommand.h"
#include <cstdint>
#include <vector>
#include <string>
#include "math/vector/Vector3.h"

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief エンティティを作成するコマンド
	class CreateEntityCommand : public IEditorCommand {
	public:
		CreateEntityCommand(std::string entityName, 
			QFE::MATH::Vector3 position, EntityManager* entityManager,
			std::string modelName = {}, bool createSprite = false);

		/// @brief エンティティを作成する。
		void Execute() override;
		/// @brief エンティティの作成を取り消す。
		void Undo() override;

	private:
		EntityManager* entityManager_;
		uint32_t entityId_;
		std::string entityName_;
		std::string modelName_;
		bool createSprite_;
		QFE::MATH::Vector3 position_;
	};
}
