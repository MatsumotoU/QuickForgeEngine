#pragma once

#include "design-patterns/component/Archive.h"

#include <algorithm>
#include <cctype>
#include <imgui/imgui.h>
#include <imgui_stdlib.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief Archive のリフレクション情報から ImGui の編集UIを生成する。
	class ImGuiArchive final : public Archive {
	public:
		ImGuiArchive(EntityManager* entityManager, uint32_t entityId);

		bool IsLoading() const override {
			return false;
		}

		void Process(const std::string& name, bool& value) override;
		void Process(const std::string& name, float& value) override;
		void Process(const std::string& name, int32_t& value) override;
		void Process(const std::string& name, uint32_t& value) override;
		void Process(const std::string& name, std::string& value) override;

		void Process(const std::string& name, MATH::Vector2& value) override;
		void Process(const std::string& name, MATH::Vector3& value) override;
		void Process(const std::string& name, MATH::Vector4& value) override;

		void Process(const std::string& name, MATH::EulerTransform& value) override;
		void Process(const std::string& name, MATH::Matrix4x4& value) override;
		void Process(const std::string& name, MATH::Bit32& value) override;
		void Process(const std::string& name, EntityReference& value) override;

	private:
		EntityManager* entityManager_;
		uint32_t entityId_;
	};
}
