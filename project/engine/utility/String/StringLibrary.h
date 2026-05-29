#pragma once
#include <stdint.h>
#include <string>
#include "engine/include/core/Memory/SafeVector.h"
#include <list>
#include <unordered_map>

namespace QFE {
	/// @brief 文字列を管理する辞書クラス
	class StringLibrary {
	public:
		StringLibrary();
		~StringLibrary();

	public:
		/// @brief 辞書を特定の名前を付けて初期化します
		void Init(const std::string& libraryFriendName);

	public:
		/// @brief 文字列を辞書に追加します
		void AddStringToLibrary(const std::string& string);
		/// @brief 文字列が辞書に存在するか探します
		bool FindString(const std::string& string);
		/// @brief 文字列のインデックスを取得します
		int32_t GetLibraryIndex(const std::string& string);
		/// @brief インデックスから文字列を取得します
		std::string GetNameFromIndex(uint32_t index);

	private:
		std::string libraryFriendryName_;
		SafeVector<std::string> library_;
	};

}
