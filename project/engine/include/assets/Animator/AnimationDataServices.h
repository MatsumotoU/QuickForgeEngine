#pragma once
#include "AnimClip.h"

namespace QFE::ANIMATION {
	/// @brief アニメーションクリップをファイルに保存する関数。(.anim)
	void SaveAnimClipToAnimFile(const AnimClip& animClip, const std::string& filePath);
	/// @brief アニメーションクリップをファイルから読み込む関数。(.anim)
	AnimClip LoadAnimClipFromAnimFile(const std::string& filePath);

}
