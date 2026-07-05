#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace QFE {
	/// @brief コンポーネントデータの抽象基底クラス
    class ComponentData {
    public:
        virtual ~ComponentData() = default;
		/// @brief コンポーネントデータをJSON形式でシリアライズする
        virtual nlohmann::json Serialize() const = 0;
		/// @brief JSON形式のデータをコンポーネントデータにデシリアライズする
        virtual void Deserialize(const nlohmann::json& json) = 0;
        /// @brief コンポーネントデータの型名を取得する
        virtual std::string GetTypeName() const = 0;
    };

}
