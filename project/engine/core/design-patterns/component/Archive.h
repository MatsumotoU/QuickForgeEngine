#pragma once
#include <string>
#include "../../math/MathInclude.h"

namespace QFE {
    /// @brief 外部からコンポーネントのメンバにアクセスするためのインターフェース
    class Archive {
    public:
        virtual ~Archive() = default;
        virtual bool IsLoading() const = 0; // 読み込み(デシリアライズ)中か判定

        // 基本型のバインディング
        virtual void Process(const std::string& name, float& value) = 0;
        virtual void Process(const std::string& name, int32_t& value) = 0;
		virtual void Process(const std::string& name, uint32_t& value) = 0;
        virtual void Process(const std::string& name, std::string& value) = 0;

		// ベクトル型のバインディング
        virtual void Process(const std::string& name, MATH::Vector2& value) = 0;
        virtual void Process(const std::string& name, MATH::Vector3& value) = 0;
		virtual void Process(const std::string& name, MATH::Vector4& value) = 0;
		virtual void Process(const std::string& name, MATH::Transform& value) = 0;
    };
}
