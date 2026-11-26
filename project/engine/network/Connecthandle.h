#pragma once
#include <cstdint>
namespace QFE::NETWORK {
	/// @brief 接続先を識別するためのハンドル.UINT32_MAXは無効なハンドルを表す
	struct ConnectHandle {
		uint32_t id = UINT32_MAX;// 接続先を識別するための一意のID
		uint32_t age = UINT32_MAX;// 接続の世代を表す値。接続が切断され再接続された場合に更新される
	};
}
