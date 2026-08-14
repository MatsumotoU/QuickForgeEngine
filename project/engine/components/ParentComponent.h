#pragma once

#include "EngineDefines.h"
#include "design-patterns/component/EntityReference.h"

namespace QFE::SCENE {
	/// @brief エンティティ間の親子関係を保持するコンポーネント。
	struct ParentComponent {
		QFE::EntityReference parent;

		[[nodiscard]] bool HasParent() const {
			return !parent.IsEmpty();
		}

		void ClearParent() {
			parent.Clear();
		}

		QFE_REFLECT_BEGIN(ParentComponent)
			QFE_REFLECT_MEMBER(parent)
		QFE_REFLECT_END()
	};

	QFE_COMPONENT(ParentComponent)
}
