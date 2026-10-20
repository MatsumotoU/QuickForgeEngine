#pragma once
#include <string>
#include <functional>
#include <memory>
#include <cstdint>
#include "core/interface/IComponentStorage.h"

namespace QFE {
	/// @brief コンポーネントストレージの制作、登録のための構造体
	struct ComponentDescriptor {
		/// シーン保存など外部データで使用する安定キー。
		std::string key;
		/// C++の型付きAPIからストレージを解決するリフレクション名。
		std::string reflectedTypeName;
		uint32_t storageIndex;
		std::function<std::unique_ptr<IComponentStorage>()> createStorageFunc;
	};
}
