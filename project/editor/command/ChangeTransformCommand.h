#pragma once
#include "IEditorCommand.h"
#include "math/transform/Transform.h"
#include <cstdint>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief EntityのTransform変更をUndo/Redo可能にするコマンド
	class ChangeTransformCommand final : public IEditorCommand {
	public:
		ChangeTransformCommand(
			uint32_t entityId,
			EntityManager* entityManager,
			const QFE::MATH::EulerTransform& before,
			const QFE::MATH::EulerTransform& after);

		void Execute() override;
		void Undo() override;

	private:
		void Apply(const QFE::MATH::EulerTransform& transform);

		uint32_t entityId_;
		EntityManager* entityManager_;
		QFE::MATH::EulerTransform before_;
		QFE::MATH::EulerTransform after_;
	};
}
