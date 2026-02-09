#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include "engine/include/core/Math/Vector/Vector3.h"

namespace QFE::Component {
	/// @brief ビルボードの種類
	enum class BillboardType {
		POINT,// カメラの位置を常に向くビルボード
		AXIAL// カメラの向きに対して特定の軸を維持するビルボード
	};

	/// @brief ビルボードコンポーネント
	class BillboardComponent : public ComponentData {
	public:
		BillboardType type_; // ビルボードの種類
		QFE::Vector3 rotateOffset_; // 差分回転量
		bool isActive_; // ビルボードの有効/無効

		BillboardComponent();
		virtual ~BillboardComponent() = default;
		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;

		std::string GetTypeName() const override { return "BillboardComponent"; }
	};
}