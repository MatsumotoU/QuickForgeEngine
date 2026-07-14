#pragma once
#include "Archive.h"
#include <nlohmann/json.hpp>

namespace QFE {
	/// @brief JSON形式でのシリアライズ/デシリアライズを行うアーカイブクラス
    class JsonArchive : public Archive {
    public:
        JsonArchive(nlohmann::json& json, bool isLoading)
            : json_(json), isLoading_(isLoading) {
        }
		// 読み込みモードか判定するための関数
        bool IsLoading() const override { return isLoading_; }

		// 各型のシリアライズ/デシリアライズ処理を実装
		void Process(const std::string& name, bool& value) override;
        void Process(const std::string& name, float& value) override;
        void Process(const std::string& name, int32_t& value) override;
		void Process(const std::string& name, uint32_t& value) override;
        void Process(const std::string& name, std::string& value) override;

		void Process(const std::string& name, MATH::Vector2& value) override;
		void Process(const std::string& name, MATH::Vector3& value) override;
		void Process(const std::string& name, MATH::Vector4& value) override;
		void Process(const std::string& name, MATH::Transform& value) override;

    private:
        nlohmann::json& json_;
        bool isLoading_;
    };
}