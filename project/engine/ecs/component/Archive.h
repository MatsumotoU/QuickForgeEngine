#pragma once
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "math/MathInclude.h"
#include "EntityReference.h"

namespace QFE {
	namespace SCENE {
		struct SplineControlPoint;
	}

    /// @brief 外部からコンポーネントのメンバにアクセスするためのインターフェース
    class Archive {
    public:
        virtual ~Archive() = default;
        virtual bool IsLoading() const = 0; // 読み込み(デシリアライズ)中か判定
		virtual bool HasField(const std::string& name) const = 0;

        // 基本型のバインディング
		virtual void Process(const std::string& name, bool& value) = 0;
        virtual void Process(const std::string& name, float& value) = 0;
        virtual void Process(const std::string& name, int32_t& value) = 0;
		virtual void Process(const std::string& name, uint32_t& value) = 0;
        virtual void Process(const std::string& name, std::string& value) = 0;
		virtual void Process(const std::string& name, std::vector<std::string>& value) = 0;

		// ベクトル型のバインディング
        virtual void Process(const std::string& name, MATH::Vector2& value) = 0;
        virtual void Process(const std::string& name, MATH::Vector3& value) = 0;
		virtual void Process(const std::string& name, MATH::Vector4& value) = 0;

		// 数学系の型のバインディング
		virtual void Process(const std::string& name, MATH::EulerTransform& value) = 0;
		virtual void Process(const std::string& name, std::vector<MATH::EulerTransform>& value) = 0;
		virtual void Process(const std::string& name, std::vector<SCENE::SplineControlPoint>& value) = 0;
        virtual void Process(const std::string& name, MATH::Matrix4x4& value) = 0;
        virtual void Process(const std::string& name, MATH::Bit32& value) = 0;
        virtual void Process(const std::string& name, EntityReference& value) = 0;

		// 可変長・構造化データのバインディング
		virtual void Process(const std::string& name, nlohmann::json& value) = 0;

		/// @brief Reflect(Archive&) を持つ任意の構造体配列を処理する。
		template <typename T>
		void Process(const std::string& name, std::vector<T>& value) {
			ProcessReflectedArray(
				name,
				T::GetTypeName(),
				value.size(),
				[&value](std::size_t size) { value.resize(size); },
				[&value](std::size_t index) {
					value.erase(value.begin() + static_cast<std::ptrdiff_t>(index));
				},
				[&value](std::size_t index, Archive& archive) {
					value[index].Reflect(archive);
				});
		}

	protected:
		using ResizeReflectedArray = std::function<void(std::size_t)>;
		using EraseReflectedArrayItem = std::function<void(std::size_t)>;
		using ReflectArrayItem = std::function<void(std::size_t, Archive&)>;

		virtual void ProcessReflectedArray(
			const std::string& name,
			const std::string& itemTypeName,
			std::size_t size,
			const ResizeReflectedArray& resize,
			const EraseReflectedArrayItem& erase,
			const ReflectArrayItem& reflect) = 0;
    };
}
