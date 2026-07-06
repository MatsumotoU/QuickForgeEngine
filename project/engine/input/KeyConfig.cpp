#include "KeyConfig.h"
#include <cassert>
#include <dinput.h>
#include <fstream>

#include "EngineDefines.h"

namespace QFE::INPUT {

	/** @brief 初期化 */
	void KeyConfig::Initialize() {
		keyMap_.clear();
		SettingDefaultKeyConfig();
	}

	/**
	 * @brief キーの追加
	 * @param name アクション名
	 * @param key キーコード(DIK_*)
	 */
	void KeyConfig::AddKey(const std::string& name, uint32_t key) {
		// 同じキーが登録されていないか確認
		for (const auto& existingKey : keyMap_[name]) {
			if (existingKey == key) {
				return;
			}
		}
		keyMap_[name].push_back(key);
	}

	/**
	 * @brief 指定されたアクション名に関連付けられた全てのキーを削除する。
	 *
	 * @param name 削除するアクション名
	 */
	void KeyConfig::RemoveKey(const std::string& name) {
		keyMap_.erase(name);
	}

	/**
	 * @brief 指定されたアクション名から特定のキーを削除する。
	 *
	 * アクション名に関連付けられたキーが全て削除された場合、そのアクション名もマップから削除されます。
	 *
	 * @param name アクション名
	 * @param key 削除するキーコード
	 */
	void KeyConfig::RemoveKey(const std::string& name, uint32_t key) {
		auto it = keyMap_.find(name);
		if (it != keyMap_.end()) {
			auto& keys = it->second;
			keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
			if (keys.empty()) {
				keyMap_.erase(it);
			}
		}
	}

	/**
	 * @brief 指定されたアクション名の特定のインデックスにあるキーを編集する。
	 *
	 * 指定されたインデックスが存在しない場合、新しいキーとして追加されます。
	 *
	 * @param name アクション名
	 * @param index 編集するキーのインデックス
	 * @param newKey 新しいキーコード
	 */
	void KeyConfig::EditKey(const std::string& name, size_t index, uint32_t newKey) {
		auto it = keyMap_.find(name);
		if (it != keyMap_.end() && index < it->second.size()) {
			it->second[index] = newKey;
		} else {
			AddKey(name, newKey);
		}
	}

	const std::vector<uint32_t>& KeyConfig::GetKeys(const std::string& name) const {
		static const std::vector<uint32_t> emptyVector;
		auto it = keyMap_.find(name);
		if (it != keyMap_.end()) {
			return it->second;
		}

#ifdef QFE::INPUT_OPTIMIZE_OFF
		QFE::INPUT_LOG("GetKeys: No keys found for action '" + name + "'", LogLevel::Error);
#endif // QFE::INPUT_OPTIMIZE_OFF
		return emptyVector;
	}

	/** @brief デフォルトのキーコンフィグを設定 */
	void KeyConfig::SettingDefaultKeyConfig() {
		// 移動
		AddKey("MoveRight", DIK_RIGHT);
		AddKey("MoveRight", DIK_D);
		AddKey("MoveLeft", DIK_LEFT);
		AddKey("MoveLeft", DIK_A);
		AddKey("MoveUp", DIK_UP);
		AddKey("MoveUp", DIK_W);
		AddKey("MoveDown", DIK_DOWN);
		AddKey("MoveDown", DIK_S);
		// ジャンプ
		AddKey("Jump", DIK_SPACE);
		// 攻撃
		AddKey("Attack", DIK_LCONTROL);
		// メニュー
		AddKey("Menu", DIK_ESCAPE);
		// 決定
		AddKey("Decide", DIK_RETURN);
		AddKey("Decide", DIK_SPACE);
		// キャンセル
		AddKey("Cancel", DIK_BACKSPACE);
		AddKey("Cancel", DIK_ESCAPE);
	}

	void KeyConfig::Finalize() {
		keyMap_.clear();
	}

	void KeyConfig::Reset() {
		keyMap_.clear();
		SettingDefaultKeyConfig();
	}
}