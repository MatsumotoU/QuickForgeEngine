#pragma once
#include "Archive.h"
#include <functional>

namespace QFE {
	/// @brief コンポーネントストレージのインターフェース
    class IComponentStorage {
    public:
        virtual ~IComponentStorage() = default;
        
        virtual void AddDefaultComponent(uint32_t entityId) = 0;
        virtual void RemoveComponent(uint32_t id) = 0;
        virtual bool HasComponent(uint32_t id) const = 0;

		virtual void Clear() = 0;

		/// @brief コンポーネントをシリアライズ/デシリアライズする
        virtual void ReflectComponent(uint32_t id, Archive& ar) = 0;
		/// @brief コンポーネントストレージの型名を取得する
        virtual std::string GetStorageTypeName() const = 0;
		/// @brief コンポーネントの生ポインタを取得する
		virtual void* GetRawPtr(uint32_t id) = 0;
    };
}
