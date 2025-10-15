#pragma once
#include <unordered_map>
#include <stdint.h>
#include <string>
#include <vector>

class KeyConfig final {
public:
	// キーコンフィグの初期化
	void Initialize();
	// 終了処理
	void Finalize();

	// キーコンフィグのリセット（デフォルトに戻す）
	void Reset();

	// キーコンフィグの保存
	void SaveKeyConfig();
	// キーコンフィグの読み込み
	void LoadKeyConfig();

	// キーコンフィグの追加
	void AddKey(const std::string& name, uint32_t key);
	// キーコンフィグの削除
	void RemoveKey(const std::string& name);
	// キーコンフィグの削除（特定のキーのみ）
	void RemoveKey(const std::string& name, uint32_t key);
	// キーコンフィグの編集（特定のキーのみ）
	void EditKey(const std::string& name, size_t index, uint32_t newKey);

	// Editor用
	// キーコンフィグの取得
	const std::vector<uint32_t>& GetKeys(const std::string& name) const;
	// mapの取得
	const std::unordered_map<std::string, std::vector<uint32_t>>& GetKeyMap() const { return keyMap_; }

private:
	void SettingDefaultKeyConfig();
	std::unordered_map<std::string, std::vector<uint32_t>> keyMap_;
};