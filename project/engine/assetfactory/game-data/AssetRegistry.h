#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace QFE::ASSET {
	/// @brief プロジェクトに登録されたアセットの永続情報。
	struct AssetMetadata {
		std::string id;
		std::string type;
		std::string name;
		std::string source;
		std::vector<std::string> dependencies;
		std::vector<std::string> previousSources; ///< 旧ファイルを自動登録し直さないための履歴。
		std::vector<std::string> embeddedImageIds; ///< GLBの画像番号に対応するアセットID。
	};

	/// @brief プロジェクト単位のアセット登録情報を管理する。
	class AssetRegistry final {
	public:
		/// @brief 登録ファイルを読み込む。未作成の場合は空の登録として開く。
		bool Open(const std::filesystem::path& projectRoot, std::string& error);
		/// @brief 登録内容を一時ファイル経由で保存する。
		bool Save(std::string& error) const;
		/// @brief 新しいアセットを追加する。重複IDや不正なパスは拒否する。
		bool Register(const AssetMetadata& asset, std::string& error);
		/// @brief 同じIDの登録情報を置き換える。
		bool Update(const AssetMetadata& asset, std::string& error);
		/// @brief IDから登録情報を検索する。存在しない場合はnullptr。
		const AssetMetadata* Find(const std::string& id) const;
		/// @brief 登録情報を取得する。変更操作の後は参照を取得し直す。
		const std::vector<AssetMetadata>& List() const { return assets_; }
	private:
		std::filesystem::path root_;
		std::vector<AssetMetadata> assets_;
	};
}
