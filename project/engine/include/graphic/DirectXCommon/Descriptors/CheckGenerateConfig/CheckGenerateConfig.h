#pragma once
#include "../Data/DescriptorGenerateConfig.h"

class CheckGenerateConfig final {
public:
	// チE��スクリプタ生�E設定をチェチE��する関数
	static bool IsValid(const DescriptorGenerateConfig& config);
};
