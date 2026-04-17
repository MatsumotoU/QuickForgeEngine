#pragma once
#include <string>
#include <vector>
#include <utility>
#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {

	/**
	 * @brief ColliderTagMaskクラス
	 * 衝突判定を行わないタグの組み合わせを管理するクラス
	 */
	class ColliderTagMask final {
	public:
		/// @brief 任意のパスからマスクするタグのテーブルを取得します。
		void Initialize(const std::string& maskTableFilePath);
		/// @brief マスクするタグのテーブルを保存して終了します.
		void Finalize();

		/// @brief タグの組み合わせを追加します.
		void AddTagMaskPair(const std::string& tag1, const std::string& tag2);
		/// @brief タグの組み合わせを削除します.
		void EraseTagMaskPair(const std::string& tag1, const std::string& tag2);
		/// @brief 指定したタグの組み合わせが衝突判定を行わないかを取得します.
		bool IsCollidable(const std::string& tag1, const std::string& tag2) const;

		/// @brief マスクするタグの組み合わせのリストを取得します.
		const std::vector<std::pair<std::string, std::string>> GetTagMaskPairs() const;
	private:
		/// @brief マスクするタグのテーブルのファイルパス
		std::string configFilePath_;
		/// @brief マスクするタグの組み合わせのリスト
		SafeVector<std::pair<std::string, std::string>> tagMaskPairs_;
	};

}
