/**
 * @file KeyConfig.h
 * @brief アクション名とキーの紐付けを管理するクラス
 */

#pragma once
#include <unordered_map>
#include <stdint.h>
#include <string>
#include <vector>

/**
 * @class KeyConfig
 * @brief キーコンフィグの保存、読み込み、およびアクションへのキー割り当てを管理するクラス
 */
class KeyConfig final {
public:
    /** @brief 初期化 */
	void Initialize();
    /** @brief 終了処理 */
	void Finalize();

    /** @brief デフォルトの設定にリセット */
	void Reset();

    /** @brief 現在の設定をファイルに保存 */
	void SaveKeyConfig();
    /** @brief 設定をファイルから読み込む */
	void LoadKeyConfig();

    /** @brief 指定アクションにキーを追加 */
	void AddKey(const std::string& name, uint32_t key);
    /** @brief 指定アクションに紐付く全てのキーを削除 */
	void RemoveKey(const std::string& name);
    /** @brief 指定アクションから特定のキーを削除 */
	void RemoveKey(const std::string& name, uint32_t key);
    /** @brief 指定アクションのキーを編集 */
	void EditKey(const std::string& name, size_t index, uint32_t newKey);

	// Editor用
    /** @brief 指定アクションに紐付くキーリストを取得 */
	const std::vector<uint32_t>& GetKeys(const std::string& name) const;
    /** @brief 全てのキーマップを取得 */
	const std::unordered_map<std::string, std::vector<uint32_t>>& GetKeyMap() const { return keyMap_; }

private:
	void SettingDefaultKeyConfig();
	std::unordered_map<std::string, std::vector<uint32_t>> keyMap_;
};
