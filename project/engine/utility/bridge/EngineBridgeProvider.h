#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/core/Bridge/IEngineBridge.h"
#include <memory>

namespace QFE::BRIDGE {
	/// @brief エンジンとエディタ間の橋渡し関数を提供するシングルトンクラス
	class EngineBridgeProvider final : public Singleton<EngineBridgeProvider> {
	public:
		/// @brief ブリッジインスタンスのセットアップ
		void SetUpBridge(std::unique_ptr<QFE::IEngineBridge> bridge);
		/// @brief ブリッジインスタンスの解放
		void FinalizeBridge();
		
		/// @brief ブリッジインスタンスの取得
		QFE::IEngineBridge* Get() const;
	private:
		std::unique_ptr<QFE::IEngineBridge> bridgeInstance_;
	};

	/// @brief ブリッジインスタンスのグローバルアクセス関数
	static inline QFE::IEngineBridge* GetBridge() {
		return EngineBridgeProvider::GetInstance()->Get();
	}
}